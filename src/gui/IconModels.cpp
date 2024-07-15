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
#include "IconModels.h"
#include "core/DatabaseIcons.h"

DefaultIconModel::DefaultIconModel(
	QObject* parent
)
	: QAbstractListModel(
		parent
	)
{
}

int DefaultIconModel::rowCount(
	const QModelIndex &parent
) const
{
	if(!parent.isValid())
	{
		return DatabaseIcons::IconCount;
	}
	return 0;
}

QVariant DefaultIconModel::data(
	const QModelIndex &index,
	const int role
) const
{
	if(!index.isValid())
	{
		return QVariant();
	}
	if(index.row() >= DatabaseIcons::IconCount)
	{
		return QVariant();
	}
	if(role == Qt::DecorationRole)
	{
		return DatabaseIcons::getInstance()->iconPixmap(
			index.row()
		);
	}
	return QVariant();
}

CustomIconModel::CustomIconModel(
	QObject* parent
)
	: QAbstractListModel(
		parent
	)
{
}

void CustomIconModel::setIcons(
	const QHash<UUID, QPixmap> &icons,
	const QList<UUID> &iconsOrder
)
{
	if(icons.count() != iconsOrder.count())
	{
		// TODO: Do an error message
		return;
	}
	this->beginResetModel();
	this->icons = icons;
	this->iconsOrder = iconsOrder;
	this->endResetModel();
}

int CustomIconModel::rowCount(
	const QModelIndex &parent
) const
{
	if(!parent.isValid())
	{
		return static_cast<int>(this->icons.size());
	}
	return 0;
}

QVariant CustomIconModel::data(
	const QModelIndex &index,
	const int role
) const
{
	if(!index.isValid())
	{
		return QVariant();
	}
	if(role == Qt::DecorationRole)
	{
		const UUID uuid_ = this->uuidFromIndex(
			index
		);
		return this->icons.value(
			uuid_
		);
	}
	return QVariant();
}

UUID CustomIconModel::uuidFromIndex(
	const QModelIndex &index
) const
{
	if(!index.isValid())
	{
		// TODO: Do an error message
		return UUID();
	}
	return this->iconsOrder.value(
		index.row()
	);
}

QModelIndex CustomIconModel::indexFromUuid(
	const UUID &uuid
) const
{
	if(const auto idx_ = static_cast<int>(this->iconsOrder.indexOf(
			uuid
		));
		idx_ > -1)
	{
		return this->index(
			idx_,
			0
		);
	}
	return QModelIndex();
}
