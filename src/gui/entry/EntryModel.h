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
#ifndef KEEPASSX_ENTRYMODEL_H
#define KEEPASSX_ENTRYMODEL_H
#include <QAbstractTableModel>
class Entry;
class Group;

class EntryModel final:public QAbstractTableModel
{
	Q_OBJECT public:
	enum ModelColumn: uint8_t
	{
		ParentGroup = 0,
		Title = 1,
		Username = 2,
		Url = 3
	};

	explicit EntryModel(
		QObject* parent = nullptr
	);
	Entry* entryFromIndex(
		const QModelIndex &index
	) const;
	QModelIndex indexFromEntry(
		Entry* entry
	) const;
	virtual int rowCount(
		const QModelIndex &parent = QModelIndex()
	) const override;
	virtual int columnCount(
		const QModelIndex &parent = QModelIndex()
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
	virtual Qt::DropActions supportedDragActions() const override;
	virtual Qt::ItemFlags flags(
		const QModelIndex &modelIndex
	) const override;
	virtual QStringList mimeTypes() const override;
	virtual QMimeData* mimeData(
		const QModelIndexList &indexes
	) const override;
	void setEntryList(
		const QList<Entry*> &entries
	);
Q_SIGNALS:
	void sig_switchedToEntryListMode();
	void sig_switchedToGroupMode();
public Q_SLOTS:
	void do_setGroup(
		Group* group
	);
private Q_SLOTS:
	void do_entryAboutToAdd(
		Entry* entry
	);
	void do_entryAdded(
		Entry* entry
	);
	void do_entryAboutToRemove(
		Entry* entry
	);
	void do_entryRemoved();
	void do_entryDataChanged(
		Entry* entry
	);
private:
	void severConnections();
	void makeConnections(
		const Group* group
	) const;
	Group* group;
	QList<Entry*> entries;
	QList<Entry*> orgEntries;
	QList<const Group*> allGroups;
};
#endif // KEEPASSX_ENTRYMODEL_H
