/*
 *  Copyright (C) 2012 Felix Geyer <debfx@fobos.de>
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
#include "EntryAttributesModel.h"
#include <algorithm>
#include "core/Entry.h"
#include "core/Tools.h"

EntryAttributesModel::EntryAttributesModel(
	QObject* parent
)
	: QAbstractListModel(
		parent
	),
	entryAttributes(
		nullptr
	),
	nextRenameDataChange(
		false
	)
{
}

void EntryAttributesModel::setEntryAttributes(
	EntryAttributes* entryAttributes
)
{
	this->beginResetModel();
	if(this->entryAttributes)
	{
		this->entryAttributes->disconnect(
			this
		);
	}
	this->entryAttributes = entryAttributes;
	if(this->entryAttributes)
	{
		this->updateAttributes();
		this->connect(
			this->entryAttributes,
			&EntryAttributes::sig_customKeyModified,
			this,
			&EntryAttributesModel::do_attributeChange
		);
		this->connect(
			this->entryAttributes,
			&EntryAttributes::sig_aboutToBeAdded,
			this,
			&EntryAttributesModel::do_attributeAboutToAdd
		);
		this->connect(
			this->entryAttributes,
			&EntryAttributes::sig_added,
			this,
			&EntryAttributesModel::do_attributeAdd
		);
		this->connect(
			this->entryAttributes,
			&EntryAttributes::sig_aboutToBeRemoved,
			this,
			&EntryAttributesModel::do_attributeAboutToRemove
		);
		this->connect(
			this->entryAttributes,
			&EntryAttributes::sig_removed,
			this,
			&EntryAttributesModel::do_attributeRemove
		);
		this->connect(
			this->entryAttributes,
			&EntryAttributes::sig_aboutToRename,
			this,
			&EntryAttributesModel::do_attributeAboutToRename
		);
		this->connect(
			this->entryAttributes,
			&EntryAttributes::sig_renamed,
			this,
			&EntryAttributesModel::do_attributeRename
		);
		this->connect(
			this->entryAttributes,
			&EntryAttributes::sig_aboutToBeReset,
			this,
			&EntryAttributesModel::do_aboutToReset
		);
		this->connect(
			this->entryAttributes,
			&EntryAttributes::sig_reset,
			this,
			&EntryAttributesModel::do_reset
		);
	}
	this->endResetModel();
}

int EntryAttributesModel::rowCount(
	const QModelIndex &parent
) const
{
	if(!this->entryAttributes || parent.isValid())
	{
		return 0;
	}
	return static_cast<int>(this->attributes.size());
}

int EntryAttributesModel::columnCount(
	const QModelIndex &parent
) const
{
	Q_UNUSED(
		parent
	);
	return 1;
}

QVariant EntryAttributesModel::headerData(
	const int section,
	const Qt::Orientation orientation,
	const int role
) const
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
	{
		return this->tr(
			"Name"
		);
	}
	return QVariant();
}

QVariant EntryAttributesModel::data(
	const QModelIndex &index,
	const int role
) const
{
	if(!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole))
	{
		return QVariant();
	}
	return this->attributes.at(
		index.row()
	);
}

bool EntryAttributesModel::setData(
	const QModelIndex &index,
	const QVariant &value,
	const int role
)
{
	if(!index.isValid() || role != Qt::EditRole || value.typeId() !=
		QMetaType::QString || value.toString().isEmpty())
	{
		return false;
	}
	const QString oldKey_ = this->attributes.at(
		index.row()
	);
	const QString newKey_ = value.toString();
	if(EntryAttributes::isDefaultAttribute(
		newKey_
	) || this->entryAttributes->getKeys().contains(
		newKey_
	))
	{
		return false;
	}
	this->entryAttributes->rename(
		oldKey_,
		newKey_
	);
	return true;
}

Qt::ItemFlags EntryAttributesModel::flags(
	const QModelIndex &index
) const
{
	if(!index.isValid())
	{
		return Qt::NoItemFlags;
	}
	return QAbstractListModel::flags(
		index
	) | Qt::ItemIsEditable;
}

QModelIndex EntryAttributesModel::indexByKey(
	const QString &key
) const
{
	const auto row_ = static_cast<int>(this->attributes.indexOf(
		key
	));
	if(row_ == -1)
	{
		return QModelIndex();
	}
	return this->index(
		row_,
		0
	);
}

QString EntryAttributesModel::keyByIndex(
	const QModelIndex &index
) const
{
	if(!index.isValid())
	{
		return QString();
	}
	return this->attributes.at(
		index.row()
	);
}

void EntryAttributesModel::do_attributeChange(
	const QString &key
)
{
	const auto row_ = static_cast<int>(this->attributes.indexOf(
		key
	));
	if(row_ == -1)
	{
		return;
	}
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

void EntryAttributesModel::do_attributeAboutToAdd(
	const QString &key
)
{
	QList<QString> rows_ = this->attributes;
	rows_.append(
		key
	);
	std::sort(
		rows_.begin(),
		rows_.end()
	);
	const auto row = static_cast<int>(rows_.indexOf(
		key
	));
	this->beginInsertRows(
		QModelIndex(),
		row,
		row
	);
}

void EntryAttributesModel::do_attributeAdd()
{
	this->updateAttributes();
	this->endInsertRows();
}

void EntryAttributesModel::do_attributeAboutToRemove(
	const QString &key
)
{
	const auto row_ = static_cast<int>(this->attributes.indexOf(
		key
	));
	this->beginRemoveRows(
		QModelIndex(),
		row_,
		row_
	);
}

void EntryAttributesModel::do_attributeRemove()
{
	this->updateAttributes();
	this->endRemoveRows();
}

void EntryAttributesModel::do_attributeAboutToRename(
	const QString &oldKey,
	const QString &newKey
)
{
	const auto oldRow_ = static_cast<int>(this->attributes.indexOf(
		oldKey
	));
	QList<QString> rows_ = this->attributes;
	rows_.removeOne(
		oldKey
	);
	rows_.append(
		newKey
	);
	std::sort(
		rows_.begin(),
		rows_.end()
	);
	auto newRow_ = static_cast<int>(rows_.indexOf(
		newKey
	));
	if(newRow_ > oldRow_)
	{
		newRow_++;
	}
	if(oldRow_ != newRow_)
	{
		const bool result_ = this->beginMoveRows(
			QModelIndex(),
			oldRow_,
			oldRow_,
			QModelIndex(),
			newRow_
		);
		Q_UNUSED(
			result_
		);
	}
	else
	{
		this->nextRenameDataChange = true;
	}
}

void EntryAttributesModel::do_attributeRename(
	const QString &oldKey,
	const QString &newKey
)
{
	Q_UNUSED(
		oldKey
	);
	this->updateAttributes();
	if(!this->nextRenameDataChange)
	{
		this->endMoveRows();
	}
	else
	{
		this->nextRenameDataChange = false;
		const QModelIndex keyIndex_ = index(
			static_cast<int>(this->attributes.indexOf(
				newKey
			)),
			0
		);
		this->dataChanged(
			keyIndex_,
			keyIndex_
		);
	}
}

void EntryAttributesModel::do_aboutToReset()
{
	this->beginResetModel();
}

void EntryAttributesModel::do_reset()
{
	this->updateAttributes();
	this->endResetModel();
}

void EntryAttributesModel::updateAttributes()
{
	this->attributes.clear();
	const QList<QString> attributesKeyList_ = this->entryAttributes->getKeys();
	for(const QString &key_: attributesKeyList_)
	{
		if(!EntryAttributes::isDefaultAttribute(
			key_
		))
		{
			this->attributes.append(
				key_
			);
		}
	}
}
