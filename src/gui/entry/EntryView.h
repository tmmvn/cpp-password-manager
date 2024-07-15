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
#ifndef KEEPASSX_ENTRYVIEW_H
#define KEEPASSX_ENTRYVIEW_H
#include <QTreeView>
#include "gui/entry/EntryModel.h"
class Entry;
class EntryModel;
class Group;
class SortFilterHideProxyModel;

class EntryView:public QTreeView
{
	Q_OBJECT public:
	explicit EntryView(
		QWidget* parent = nullptr
	);
	virtual void setModel(
		QAbstractItemModel* model
	) override;
	Entry* getCurrentEntry() const;
	void setCurrentEntry(
		Entry* entry
	) const;
	Entry* entryFromIndex(
		const QModelIndex &index
	) const;
	void setEntryList(
		const QList<Entry*> &entries
	);
	bool isInEntryListMode() const;
	qsizetype numberOfSelectedEntries() const;
	void setFirstEntryActive();

	EntryModel* getModel() const
	{
		return model;
	};
public Q_SLOTS:
	void do_setGroup(
		Group* group
	);
	void do_setFocus();
Q_SIGNALS:
	void sig_entryActivated(
		Entry* entry,
		EntryModel::ModelColumn column
	);
	void sig_entrySelectionChanged();
protected:
	virtual void keyPressEvent(
		QKeyEvent* event
	) override;
private Q_SLOTS:
	void do_emitEntryActivated(
		const QModelIndex &index
	);
	void do_switchToEntryListMode();
	void do_switchToGroupMode();
private:
	EntryModel* const model;
	SortFilterHideProxyModel* const sortModel;
	bool inEntryListMode;
};
#endif // KEEPASSX_ENTRYVIEW_H
