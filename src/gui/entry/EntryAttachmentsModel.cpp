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
#include "EntryAttachmentsModel.h"
#include <algorithm>
#include "core/Entry.h"
#include "core/Tools.h"

EntryAttachmentsModel::EntryAttachmentsModel(
	QObject* parent
)
	: QAbstractListModel(
		parent
	),
	entryAttachments(
		nullptr
	)
{
}

void EntryAttachmentsModel::setEntryAttachments(
	EntryAttachments* entryAttachments
)
{
	this->beginResetModel();
	if(this->entryAttachments)
	{
		this->entryAttachments->disconnect(
			this
		);
	}
	this->entryAttachments = entryAttachments;
	if(this->entryAttachments)
	{
		this->connect(
			this->entryAttachments,
			&EntryAttachments::sig_keyModified,
			this,
			&EntryAttachmentsModel::do_attachmentChange
		);
		this->connect(
			this->entryAttachments,
			&EntryAttachments::sig_aboutToBeAdded,
			this,
			&EntryAttachmentsModel::do_attachmentAboutToAdd
		);
		this->connect(
			this->entryAttachments,
			&EntryAttachments::sig_added,
			this,
			&EntryAttachmentsModel::do_attachmentAdd
		);
		this->connect(
			this->entryAttachments,
			&EntryAttachments::sig_aboutToBeRemoved,
			this,
			&EntryAttachmentsModel::do_attachmentAboutToRemove
		);
		this->connect(
			this->entryAttachments,
			&EntryAttachments::sig_removed,
			this,
			&EntryAttachmentsModel::do_attachmentRemove
		);
		this->connect(
			this->entryAttachments,
			&EntryAttachments::sig_aboutToBeReset,
			this,
			&EntryAttachmentsModel::do_aboutToReset
		);
		this->connect(
			this->entryAttachments,
			&EntryAttachments::sig_reset,
			this,
			&EntryAttachmentsModel::do_reset
		);
	}
	this->endResetModel();
}

int EntryAttachmentsModel::rowCount(
	const QModelIndex &parent
) const
{
	if(!this->entryAttachments || parent.isValid())
	{
		return 0;
	}
	return static_cast<int>(this->entryAttachments->getKeys().size());
}

int EntryAttachmentsModel::columnCount(
	const QModelIndex &parent
) const
{
	Q_UNUSED(
		parent
	);
	return 1;
}

QVariant EntryAttachmentsModel::data(
	const QModelIndex &index,
	const int role
) const
{
	if(!index.isValid())
	{
		return QVariant();
	}
	if(role == Qt::DisplayRole && index.column() == 0)
	{
		QString key_ = keyByIndex(
			index
		);
		return QString(
			"%1 (%2)"
		).arg(
			key_,
			Tools::humanReadableFileSize(
				this->entryAttachments->getValue(
					key_
				).size()
			)
		);
	}
	return QVariant();
}

QString EntryAttachmentsModel::keyByIndex(
	const QModelIndex &index
) const
{
	if(!index.isValid())
	{
		return QString();
	}
	return this->entryAttachments->getKeys().at(
		index.row()
	);
}

void EntryAttachmentsModel::do_attachmentChange(
	const QString &key
)
{
	const int row_ = static_cast<int>(this->entryAttachments->getKeys().indexOf(
		key
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

void EntryAttachmentsModel::do_attachmentAboutToAdd(
	const QString &key
)
{
	QList<QString> rows_ = this->entryAttachments->getKeys();
	rows_.append(
		key
	);
	std::sort(
		rows_.begin(),
		rows_.end()
	);
	const int row_ = static_cast<int>(rows_.indexOf(
		key
	));
	this->beginInsertRows(
		QModelIndex(),
		row_,
		row_
	);
}

void EntryAttachmentsModel::do_attachmentAdd()
{
	this->endInsertRows();
}

void EntryAttachmentsModel::do_attachmentAboutToRemove(
	const QString &key
)
{
	const int row_ = static_cast<int>(this->entryAttachments->getKeys().indexOf(
		key
	));
	this->beginRemoveRows(
		QModelIndex(),
		row_,
		row_
	);
}

void EntryAttachmentsModel::do_attachmentRemove()
{
	this->endRemoveRows();
}

void EntryAttachmentsModel::do_aboutToReset()
{
	this->beginResetModel();
}

void EntryAttachmentsModel::do_reset()
{
	this->endResetModel();
}
