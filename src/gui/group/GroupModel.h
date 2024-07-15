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
#ifndef KEEPASSX_GROUPMODEL_H
#define KEEPASSX_GROUPMODEL_H
#include <QAbstractItemModel>
class Database;
class Group;

class GroupModel final:public QAbstractItemModel
{
	Q_OBJECT public:
	explicit GroupModel(
		Database* db,
		QObject* parent = nullptr
	);
	void changeDatabase(
		Database* newDb
	);
	QModelIndex index(
		Group* group
	) const;
	static Group* groupFromIndex(
		const QModelIndex &index
	);
	virtual int rowCount(
		const QModelIndex &parent = QModelIndex()
	) const override;
	virtual int columnCount(
		const QModelIndex &parent = QModelIndex()
	) const override;
	virtual QModelIndex index(
		int row,
		int column,
		const QModelIndex &parent = QModelIndex()
	) const override;
	virtual QModelIndex parent(
		const QModelIndex &index
	) const override;
	virtual QVariant data(
		const QModelIndex &index,
		int role = Qt::DisplayRole
	) const override;
	virtual QVariant headerData(
		int section,
		Qt::Orientation orientation,
		int role = Qt::DisplayRole
	) const override;
	virtual Qt::DropActions supportedDropActions() const override;
	virtual Qt::ItemFlags flags(
		const QModelIndex &modelIndex
	) const override;
	virtual bool dropMimeData(
		const QMimeData* data,
		Qt::DropAction action,
		int row,
		int column,
		const QModelIndex &parent
	) override;
	virtual QStringList mimeTypes() const override;
	virtual QMimeData* mimeData(
		const QModelIndexList &indexes
	) const override;
private:
	QModelIndex parent(
		Group* group
	) const;
private Q_SLOTS:
	void do_groupDataChanged(
		Group* group
	);
	void do_groupAboutToRemove(
		Group* group
	);
	void do_groupRemoved();
	void do_groupAboutToAdd(
		Group* group,
		int index
	);
	void do_groupAdded();
	void do_groupAboutToMove(
		Group* group,
		Group* toGroup,
		int pos
	);
	void do_groupMoved();
private:
	Database* db;
};
#endif // KEEPASSX_GROUPMODEL_H
