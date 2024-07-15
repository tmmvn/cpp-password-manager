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
#include "EntryHistoryModel.h"
#include "core/Entry.h"
#include "core/Global.h"

EntryHistoryModel::EntryHistoryModel(
	QObject* parent
)
	: QAbstractTableModel(
		parent
	)
{
}

Entry* EntryHistoryModel::entryFromIndex(
	const QModelIndex &index
) const
{
	if(!index.isValid())
	{
		return nullptr;
	}
	if(index.row() >= this->historyEntries.size())
	{
		return nullptr;
	}
	return this->historyEntries.at(
		index.row()
	);
}

int EntryHistoryModel::columnCount(
	const QModelIndex &parent
) const
{
	Q_UNUSED(
		parent
	);
	return 4;
}

int EntryHistoryModel::rowCount(
	const QModelIndex &parent
) const
{
	if(!parent.isValid())
	{
		return static_cast<int>(this->historyEntries.count());
	}
	return 0;
}

QVariant EntryHistoryModel::data(
	const QModelIndex &index,
	const int role
) const
{
	if(!index.isValid())
	{
		return QVariant();
	}
	if(role == Qt::DisplayRole || role == Qt::UserRole)
	{
		const Entry* entry_ = entryFromIndex(
			index
		);
		const TimeInfo timeInfo_ = entry_->getTimeInfo();
		QDateTime lastModificationLocalTime_ = timeInfo_.
			getLastModificationTime().toLocalTime();
		switch(index.column())
		{
			case 0:
			{
				if(role == Qt::DisplayRole)
				{
					return lastModificationLocalTime_.toString(
						Qt::ISODateWithMs
					);
				}
				return lastModificationLocalTime_;
			}
			case 1:
				return entry_->getTitle();
			case 2:
				return entry_->getUsername();
			case 3:
				return entry_->getURL();
			default:
				break;
		}
	}
	return QVariant();
}

QVariant EntryHistoryModel::headerData(
	const int section,
	const Qt::Orientation orientation,
	const int role
) const
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)
		{
			case 0:
				return this->tr(
					"Last modified"
				);
			case 1:
				return this->tr(
					"Title"
				);
			case 2:
				return this->tr(
					"Username"
				);
			case 3:
				return this->tr(
					"URL"
				);
			default:
				break;
		}
	}
	return QVariant();
}

void EntryHistoryModel::setEntries(
	const QList<Entry*> &entries
)
{
	this->beginResetModel();
	this->historyEntries = entries;
	this->deletedHistoryEntries.clear();
	this->endResetModel();
}

void EntryHistoryModel::clear()
{
	this->beginResetModel();
	this->historyEntries.clear();
	this->deletedHistoryEntries.clear();
	this->endResetModel();
}

QList<Entry*> EntryHistoryModel::deletedEntries()
{
	return this->deletedHistoryEntries;
}

void EntryHistoryModel::deleteIndex(
	const QModelIndex &index
)
{
	if(index.isValid())
	{
		Entry* entry_ = this->entryFromIndex(
			index
		);
		this->beginRemoveRows(
			QModelIndex(),
			static_cast<int>(this->historyEntries.indexOf(
				entry_
			)),
			static_cast<int>(this->historyEntries.indexOf(
				entry_
			))
		);
		this->historyEntries.removeAll(
			entry_
		);
		this->deletedHistoryEntries << entry_;
		this->endRemoveRows();
	}
}

void EntryHistoryModel::deleteAll()
{
	if(this->historyEntries.count() <= 0)
	{
		return;
	}
	this->beginRemoveRows(
		QModelIndex(),
		0,
		static_cast<int>(this->historyEntries.size() - 1)
	);
	for(Entry* entry_: asConst(
			this->historyEntries
		))
	{
		this->deletedHistoryEntries << entry_;
	}
	this->historyEntries.clear();
	this->endRemoveRows();
}
