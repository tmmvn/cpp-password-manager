/*
 *  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "EntryModel.h"
#include <QFont>
#include <QIODevice>
#include <QMimeData>
#include "core/DatabaseIcons.h"
#include "core/Entry.h"
#include "core/Global.h"
#include "core/Group.h"
#include "core/Metadata.h"

EntryModel::EntryModel(
	QObject* parent
)
	: QAbstractTableModel(
		parent
	),
	group(
		nullptr
	)
{
}

Entry* EntryModel::entryFromIndex(
	const QModelIndex &index
) const
{
	if(!index.isValid())
	{
		return nullptr;
	}
	if(index.row() >= this->entries.size())
	{
		return nullptr;
	}
	return this->entries.at(
		index.row()
	);
}

QModelIndex EntryModel::indexFromEntry(
	Entry* entry
) const
{
	const auto row_ = static_cast<int>(this->entries.indexOf(
		entry
	));
	if(row_ == -1)
	{
		return QModelIndex();
	}
	return index(
		row_,
		1
	);
}

void EntryModel::do_setGroup(
	Group* group
)
{
	if(!group || this->group == group)
	{
		return;
	}
	this->beginResetModel();
	this->severConnections();
	this->group = group;
	this->allGroups.clear();
	this->entries = group->getEntries();
	this->orgEntries.clear();
	this->makeConnections(
		this->group
	);
	this->endResetModel();
	 this->sig_switchedToGroupMode();
}

void EntryModel::setEntryList(
	const QList<Entry*> &entries
)
{
	this->beginResetModel();
	this->severConnections();
	this->group = nullptr;
	this->allGroups.clear();
	this->entries = entries;
	this->orgEntries = entries;
	QSet<Database*> databases_;
	for(Entry* entry_: asConst(
			this->entries
		))
	{
		databases_.insert(
			entry_->getGroup()->getDatabase()
		);
	}
	for(Database* db_: asConst(
			databases_
		))
	{
		if(!db_)
		{
			continue;
		}
		const QList<Group*> groupList_ = db_->getRootGroup()->
			getGroupsRecursive(
				true
			);
		for(const Group* group_: groupList_)
		{
			this->allGroups.append(
				group_
			);
		}
		if(db_->getMetadata()->getRecycleBin())
		{
			this->allGroups.removeOne(
				db_->getMetadata()->getRecycleBin()
			);
		}
	}
	for(const Group* group_: asConst(
			this->allGroups
		))
	{
		this->makeConnections(
			group_
		);
	}
	this->endResetModel();
	 this->sig_switchedToEntryListMode();
}

int EntryModel::rowCount(
	const QModelIndex &parent
) const
{
	if(parent.isValid())
	{
		return 0;
	}
	return static_cast<int>(this->entries.size());
}

int EntryModel::columnCount(
	const QModelIndex &parent
) const
{
	Q_UNUSED(
		parent
	);
	return 4;
}

QVariant EntryModel::data(
	const QModelIndex &index,
	const int role
) const
{
	if(!index.isValid())
	{
		return QVariant();
	}
	Entry* entry_ = this->entryFromIndex(
		index
	);
	if(role == Qt::DisplayRole)
	{
		switch(index.column())
		{
			case ParentGroup:
				if(entry_->getGroup())
				{
					return entry_->getGroup()->getName();
				}
				break;
			case Title:
				return entry_->getTitle();
			case Username:
				return entry_->getUsername();
			case Url:
				return entry_->getURL();
			default:
				break;
		}
	}
	else if(role == Qt::DecorationRole)
	{
		switch(index.column())
		{
			case ParentGroup:
				if(entry_->getGroup())
				{
					return entry_->getGroup()->getIconScaledPixmap();
				}
				break;
			case Title:
				if(entry_->isExpired())
				{
					return DatabaseIcons::getInstance()->iconPixmap(
						DatabaseIcons::ExpiredIconIndex
					);
				}
				return entry_->getIconScaledPixmap();
			default:
				break;
		}
	}
	else if(role == Qt::FontRole)
	{
		QFont font_;
		if(entry_->isExpired())
		{
			font_.setStrikeOut(
				true
			);
		}
		return font_;
	}
	return QVariant();
}

QVariant EntryModel::headerData(
	const int section,
	const Qt::Orientation orientation,
	const int role
) const
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)
		{
			case ParentGroup:
				return this->tr(
					"Group"
				);
			case Title:
				return this->tr(
					"Title"
				);
			case Username:
				return this->tr(
					"Username"
				);
			case Url:
				return this->tr(
					"URL"
				);
			default:
				break;
		}
	}
	return QVariant();
}

Qt::DropActions EntryModel::supportedDropActions() const
{
	return Qt::IgnoreAction;
}

Qt::DropActions EntryModel::supportedDragActions() const
{
	return (Qt::MoveAction | Qt::CopyAction);
}

Qt::ItemFlags EntryModel::flags(
	const QModelIndex &modelIndex
) const
{
	if(!modelIndex.isValid())
	{
		return Qt::NoItemFlags;
	}
	return QAbstractTableModel::flags(
		modelIndex
	) | Qt::ItemIsDragEnabled;
}

QStringList EntryModel::mimeTypes() const
{
	QStringList types_;
	types_ << QLatin1String(
		"application/x-keepassx-entry"
	);
	return types_;
}

QMimeData* EntryModel::mimeData(
	const QModelIndexList &indexes
) const
{
	if(indexes.isEmpty())
	{
		return nullptr;
	}
	const auto data_ = new QMimeData();
	QByteArray encoded_;
	QDataStream stream_(
		&encoded_,
		QIODevice::WriteOnly
	);
	QSet<Entry*> seenEntries_;
	for(const QModelIndex &index_: indexes)
	{
		if(!index_.isValid())
		{
			continue;
		}
		if(Entry* entry_ = this->entryFromIndex(
				index_
			);
			!seenEntries_.contains(
				entry_
			))
		{
			// make sure we don't add entries multiple times when we get indexes
			// with the same row but different columns
			stream_ << entry_->getGroup()->getDatabase()->getUUID() << entry_->
				getUUID();
			seenEntries_.insert(
				entry_
			);
		}
	}
	if(seenEntries_.isEmpty())
	{
		delete data_;
		return nullptr;
	}
	data_->setData(
		mimeTypes().at(
			0
		),
		encoded_
	);
	return data_;
}

void EntryModel::do_entryAboutToAdd(
	Entry* entry
)
{
	if(!this->group && !this->orgEntries.contains(
		entry
	))
	{
		return;
	}
	this->beginInsertRows(
		QModelIndex(),
		static_cast<int>(this->entries.size()),
		static_cast<int>(this->entries.size())
	);
	if(!this->group)
	{
		this->entries.append(
			entry
		);
	}
}

void EntryModel::do_entryAdded(
	Entry* entry
)
{
	if(!this->group && !this->orgEntries.contains(
		entry
	))
	{
		return;
	}
	if(this->group)
	{
		this->entries = this->group->getEntries();
	}
	this->endInsertRows();
}

void EntryModel::do_entryAboutToRemove(
	Entry* entry
)
{
	this->beginRemoveRows(
		QModelIndex(),
		static_cast<int>(this->entries.indexOf(
			entry
		)),
		static_cast<int>(this->entries.indexOf(
			entry
		))
	);
	if(!this->group)
	{
		this->entries.removeAll(
			entry
		);
	}
}

void EntryModel::do_entryRemoved()
{
	if(this->group)
	{
		this->entries = this->group->getEntries();
	}
	this->endRemoveRows();
}

void EntryModel::do_entryDataChanged(
	Entry* entry
)
{
	const auto row_ = static_cast<int>(this->entries.indexOf(
		entry
	));
	 this->dataChanged(
		index(
			row_,
			0
		),
		index(
			row_,
			this->columnCount() - 1
		)
	);
}

void EntryModel::severConnections()
{
	if(this->group)
	{
		this->disconnect(
			this->group,
			nullptr,
			this,
			nullptr
		);
	}
	for(const Group* group_: asConst(
			this->allGroups
		))
	{
		this->disconnect(
			group_,
			nullptr,
			this,
			nullptr
		);
	}
}

void EntryModel::makeConnections(
	const Group* group
) const
{
	this->connect(
		group,
		&Group::sig_entryAboutToAdd,
		this,
		&EntryModel::do_entryAboutToAdd
	);
	this->connect(
		group,
		&Group::sig_entryAdded,
		this,
		&EntryModel::do_entryAdded
	);
	this->connect(
		group,
		&Group::sig_entryAboutToRemove,
		this,
		&EntryModel::do_entryAboutToRemove
	);
	this->connect(
		group,
		&Group::sig_entryRemoved,
		this,
		&EntryModel::do_entryRemoved
	);
	this->connect(
		group,
		&Group::sig_entryDataChanged,
		this,
		&EntryModel::do_entryDataChanged
	);
}
