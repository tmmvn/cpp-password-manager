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
#ifndef KEEPASSX_ICONMODELS_H
#define KEEPASSX_ICONMODELS_H
#include <QAbstractListModel>
#include <QPixmap>
#include "core/UUID.h"

class DefaultIconModel final:public QAbstractListModel
{
	Q_OBJECT public:
	explicit DefaultIconModel(
		QObject* parent = nullptr
	);
	virtual int rowCount(
		const QModelIndex &parent = QModelIndex()
	) const override;
	virtual QVariant data(
		const QModelIndex &index,
		int role = Qt::DisplayRole
	) const override;
};

class CustomIconModel final:public QAbstractListModel
{
	Q_OBJECT public:
	explicit CustomIconModel(
		QObject* parent = nullptr
	);
	virtual int rowCount(
		const QModelIndex &parent = QModelIndex()
	) const override;
	virtual QVariant data(
		const QModelIndex &index,
		int role = Qt::DisplayRole
	) const override;
	void setIcons(
		const QHash<UUID, QPixmap> &icons,
		const QList<UUID> &iconsOrder
	);
	UUID uuidFromIndex(
		const QModelIndex &index
	) const;
	QModelIndex indexFromUuid(
		const UUID &uuid
	) const;
private:
	QHash<UUID, QPixmap> icons;
	QList<UUID> iconsOrder;
};
#endif // KEEPASSX_ICONMODELS_H
