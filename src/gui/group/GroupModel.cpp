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
#include "GroupModel.h"
#include <QFont>
#include <QIODevice>
#include <QMimeData>
#include "core/Database.h"
#include "core/DatabaseIcons.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "core/Tools.h"

GroupModel::GroupModel(
	Database* db,
	QObject* parent
)
	: QAbstractItemModel(
		parent
	),
	db(
		nullptr
	)
{
	this->changeDatabase(
		db
	);
}

void GroupModel::changeDatabase(
	Database* newDb
)
{
	this->beginResetModel();
	if(this->db)
	{
		this->db->disconnect(
			this
		);
	}
	this->db = newDb;
	this->connect(
		this->db,
		&Database::sig_groupDataChanged,
		this,
		&GroupModel::do_groupDataChanged
	);
	this->connect(
		this->db,
		&Database::sig_groupAboutToAdd,
		this,
		&GroupModel::do_groupAboutToAdd
	);
	this->connect(
		this->db,
		&Database::sig_groupAdded,
		this,
		&GroupModel::do_groupAdded
	);
	this->connect(
		this->db,
		&Database::sig_groupAboutToRemove,
		this,
		&GroupModel::do_groupAboutToRemove
	);
	this->connect(
		this->db,
		&Database::sig_groupRemoved,
		this,
		&GroupModel::do_groupRemoved
	);
	this->connect(
		this->db,
		&Database::sig_groupAboutToMove,
		this,
		&GroupModel::do_groupAboutToMove
	);
	this->connect(
		this->db,
		&Database::sig_groupMoved,
		this,
		&GroupModel::do_groupMoved
	);
	this->endResetModel();
}

int GroupModel::rowCount(
	const QModelIndex &parent
) const
{
	if(!parent.isValid())
	{
		// we have exactly 1 root item
		return 1;
	}
	const Group* group_ = this->groupFromIndex(
		parent
	);
	return static_cast<int>(group_->getChildren().size());
}

int GroupModel::columnCount(
	const QModelIndex &parent
) const
{
	Q_UNUSED(
		parent
	);
	return 1;
}

QModelIndex GroupModel::index(
	const int row,
	const int column,
	const QModelIndex &parent
) const
{
	if(!this->hasIndex(
		row,
		column,
		parent
	))
	{
		return QModelIndex();
	}
	Group* group_;
	if(!parent.isValid())
	{
		group_ = this->db->getRootGroup();
	}
	else
	{
		group_ = this->groupFromIndex(
			parent
		)->getChildren().at(
			row
		);
	}
	return this->createIndex(
		row,
		column,
		group_
	);
}

QModelIndex GroupModel::parent(
	const QModelIndex &index
) const
{
	if(!index.isValid())
	{
		return QModelIndex();
	}
	return this->parent(
		this->groupFromIndex(
			index
		)
	);
}

QModelIndex GroupModel::parent(
	Group* group
) const
{
	Group* parentGroup_ = group->getParentGroup();
	if(!parentGroup_)
	{
		// index is already the root group
		return QModelIndex();
	}
	const Group* grandParentGroup_ = parentGroup_->getParentGroup();
	if(!grandParentGroup_)
	{
		// parent is the root group
		return this->createIndex(
			0,
			0,
			parentGroup_
		);
	}
	return this->createIndex(
		static_cast<int>(grandParentGroup_->getChildren().indexOf(
			parentGroup_
		)),
		0,
		parentGroup_
	);
}

QVariant GroupModel::data(
	const QModelIndex &index,
	const int role
) const
{
	if(!index.isValid())
	{
		return QVariant();
	}
	const Group* group_ = this->groupFromIndex(
		index
	);
	if(role == Qt::DisplayRole)
	{
		return group_->getName();
	}
	if(role == Qt::DecorationRole)
	{
		if(group_->isExpired())
		{
			return DatabaseIcons::getInstance()->iconPixmap(
				DatabaseIcons::ExpiredIconIndex
			);
		}
		return group_->getIconScaledPixmap();
	}
	if(role == Qt::FontRole)
	{
		QFont font_;
		if(group_->isExpired())
		{
			font_.setStrikeOut(
				true
			);
		}
		return font_;
	}
	return QVariant();
}

QVariant GroupModel::headerData(
	const int section,
	const Qt::Orientation orientation,
	const int role
) const
{
	Q_UNUSED(
		section
	);
	Q_UNUSED(
		orientation
	);
	Q_UNUSED(
		role
	);
	return QVariant();
}

QModelIndex GroupModel::index(
	Group* group
) const
{
	int row_;
	if(!group->getParentGroup())
	{
		row_ = 0;
	}
	else
	{
		row_ = static_cast<int>(group->getParentGroup()->getChildren().indexOf(
			group
		));
	}
	return this->createIndex(
		row_,
		0,
		group
	);
}

Group* GroupModel::groupFromIndex(
	const QModelIndex &index
)
{
	if(!index.internalPointer())
	{
		return nullptr;
	}
	return static_cast<Group*>(index.internalPointer());
}

Qt::DropActions GroupModel::supportedDropActions() const
{
	return Qt::MoveAction | Qt::CopyAction;
}

Qt::ItemFlags GroupModel::flags(
	const QModelIndex &modelIndex
) const
{
	if(!modelIndex.isValid())
	{
		return Qt::NoItemFlags;
	}
	if(modelIndex == index(
		0,
		0
	))
	{
		return QAbstractItemModel::flags(
			modelIndex
		) | Qt::ItemIsDropEnabled;
	}
	return QAbstractItemModel::flags(
		modelIndex
	) | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
}

bool GroupModel::dropMimeData(
	const QMimeData* data,
	const Qt::DropAction action,
	int row,
	const int column,
	const QModelIndex &parent
)
{
	Q_UNUSED(
		column
	);
	if(action == Qt::IgnoreAction)
	{
		return true;
	}
	if(!data || (action != Qt::MoveAction && action != Qt::CopyAction) || !
		parent.isValid())
	{
		return false;
	}
	// check if the format is supported
	const QStringList types_ = mimeTypes();
	if(types_.size() != 2)
	{
		return false;
	}
	const bool isGroup_ = data->hasFormat(
		types_.at(
			0
		)
	);
	if(const bool isEntry_ = data->hasFormat(
			types_.at(
				1
			)
		);
		!isGroup_ && !isEntry_)
	{
		return false;
	}
	if(row > rowCount(
		parent
	))
	{
		row = rowCount(
			parent
		);
	}
	// decode and insert
	QByteArray encoded_ = data->data(
		isGroup_
		? types_.at(
			0
		)
		: types_.at(
			1
		)
	);
	QDataStream stream_(
		&encoded_,
		QIODevice::ReadOnly
	);
	Group* parentGroup_ = this->groupFromIndex(
		parent
	);
	if(isGroup_)
	{
		UUID dbUuid_;
		UUID groupUuid_;
		stream_ >> dbUuid_ >> groupUuid_;
		Database* db_ = Database::databaseByUUID(
			dbUuid_
		);
		if(!db_)
		{
			return false;
		}
		Group* dragGroup_ = db_->resolveGroup(
			groupUuid_
		);
		if(!dragGroup_ || !Tools::hasChild(
			db_,
			dragGroup_
		) || dragGroup_ == db_->getRootGroup())
		{
			return false;
		}
		if(dragGroup_ == parentGroup_ || Tools::hasChild(
			dragGroup_,
			parentGroup_
		))
		{
			return false;
		}
		if(parentGroup_ == dragGroup_->getParentGroup() && row > parentGroup_->
			getChildren().indexOf(
				dragGroup_
			))
		{
			row--;
		}
		Group* group_;
		if(action == Qt::MoveAction)
		{
			group_ = dragGroup_;
		}
		else
		{
			group_ = dragGroup_->clone();
		}
		Database* sourceDb_ = dragGroup_->getDatabase();
		if(Database* targetDb_ = parentGroup_->getDatabase();
			sourceDb_ != targetDb_)
		{
			const QSet<UUID> customIcons_ = group_->getCustomIconsRecursive();
			targetDb_->getMetadata()->copyCustomIcons(
				customIcons_,
				sourceDb_->getMetadata()
			);
		}
		group_->setParent(
			parentGroup_,
			row
		);
	}
	else
	{
		if(row != -1)
		{
			return false;
		}
		while(!stream_.atEnd())
		{
			UUID dbUuid_;
			UUID entryUuid_;
			stream_ >> dbUuid_ >> entryUuid_;
			Database* db_ = Database::databaseByUUID(
				dbUuid_
			);
			if(!db_)
			{
				continue;
			}
			Entry* dragEntry_ = db_->resolveEntry(
				entryUuid_
			);
			if(!dragEntry_ || !Tools::hasChild(
				db_,
				dragEntry_
			))
			{
				continue;
			}
			Entry* entry_;
			if(action == Qt::MoveAction)
			{
				entry_ = dragEntry_;
			}
			else
			{
				entry_ = dragEntry_->clone(
					Entry::CloneNewUuid | Entry::CloneResetTimeInfo
				);
			}
			Database* sourceDb_ = dragEntry_->getGroup()->getDatabase();
			Database* targetDb_ = parentGroup_->getDatabase();
			if(UUID customIcon_ = entry_->getIconUUID();
				sourceDb_ != targetDb_ && !customIcon_.isNull() && !targetDb_->
				getMetadata()->containsCustomIcon(
					customIcon_
				))
			{
				targetDb_->getMetadata()->addCustomIcon(
					customIcon_,
					sourceDb_->getMetadata()->getCustomIcon(
						customIcon_
					)
				);
			}
			entry_->setGroup(
				parentGroup_
			);
		}
	}
	return true;
}

QStringList GroupModel::mimeTypes() const
{
	QStringList types_;
	types_ << "application/x-keepassx-group";
	types_ << "application/x-keepassx-entry";
	return types_;
}

QMimeData* GroupModel::mimeData(
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
	QSet<Group*> seenGroups_;
	for(const QModelIndex &index_: indexes)
	{
		if(!index_.isValid())
		{
			continue;
		}
		if(Group* group_ = this->groupFromIndex(
				index_
			);
			!seenGroups_.contains(
				group_
			))
		{
			// make sure we don't add groups multiple times when we get indexes
			// with the same row but different columns
			stream_ << this->db->getUUID() << group_->getUUID();
			seenGroups_.insert(
				group_
			);
		}
	}
	if(seenGroups_.isEmpty())
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

void GroupModel::do_groupDataChanged(
	Group* group
)
{
	const QModelIndex ix_ = this->index(
		group
	);
	this->dataChanged(
		ix_,
		ix_
	);
}

void GroupModel::do_groupAboutToRemove(
	Group* group
)
{
	if(!group->getParentGroup())
	{
		return;
	}
	const QModelIndex parentIndex_ = parent(
		group
	);
	if(!parentIndex_.isValid())
	{
		return;
	}
	const auto pos_ = static_cast<int>(group->getParentGroup()->getChildren().
		indexOf(
			group
		));
	if(pos_ == -1)
	{
		return;
	}
	this->beginRemoveRows(
		parentIndex_,
		pos_,
		pos_
	);
}

void GroupModel::do_groupRemoved()
{
	this->endRemoveRows();
}

void GroupModel::do_groupAboutToAdd(
	Group* group,
	const int index
)
{
	if(!group->getParentGroup())
	{
		return;
	}
	const QModelIndex parentIndex_ = parent(
		group
	);
	this->beginInsertRows(
		parentIndex_,
		index,
		index
	);
}

void GroupModel::do_groupAdded()
{
	this->endInsertRows();
}

void GroupModel::do_groupAboutToMove(
	Group* group,
	Group* toGroup,
	int pos
)
{
	if(!group->getParentGroup())
	{
		return;
	}
	const QModelIndex oldParentIndex_ = parent(
		group
	);
	const QModelIndex newParentIndex_ = this->index(
		toGroup
	);
	const auto oldPos_ = static_cast<int>(group->getParentGroup()->getChildren()
		.indexOf(
			group
		));
	if(group->getParentGroup() == toGroup && pos > oldPos_)
	{
		// beginMoveRows() has a bit different semantics than Group::setParent() and
		// QList::move() when the new position is greater than the old
		pos++;
	}
	const bool moveResult_ = this->beginMoveRows(
		oldParentIndex_,
		oldPos_,
		oldPos_,
		newParentIndex_,
		pos
	);
	Q_UNUSED(
		moveResult_
	);
}

void GroupModel::do_groupMoved()
{
	this->endMoveRows();
}
