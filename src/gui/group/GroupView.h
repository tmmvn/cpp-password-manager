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
#ifndef KEEPASSX_GROUPVIEW_H
#define KEEPASSX_GROUPVIEW_H
#include <QTreeView>
class Database;
class Group;
class GroupModel;

class GroupView final:public QTreeView
{
	Q_OBJECT public:
	explicit GroupView(
		Database* db,
		QWidget* parent = nullptr
	);
	void changeDatabase(
		Database* newDb
	) const;
	virtual void setModel(
		QAbstractItemModel* model
	) override;
	Group* getCurrentGroup() const;
	void setCurrentGroup(
		Group* group
	);
	void expandGroup(
		Group* group,
		bool expand = true
	);

	GroupModel* getModel() const
	{
		return model;
	}

Q_SIGNALS:
	void sig_groupChanged(
		Group* group
	);
private Q_SLOTS:
	void do_expandedChanged(
		const QModelIndex &index
	) const;
	void do_emitGroupChanged();
	void do_syncExpandedState(
		const QModelIndex &parent,
		int start,
		int end
	);
	void do_modelReset();
protected:
	virtual void dragMoveEvent(
		QDragMoveEvent* event
	) override;
private:
	void recInitExpanded(
		Group* group
	);
	void emitGroupChanged(
		const QModelIndex &index
	);
	GroupModel* const model;
	bool updatingExpanded;
};
#endif // KEEPASSX_GROUPVIEW_H
