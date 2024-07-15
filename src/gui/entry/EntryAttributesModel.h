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
#ifndef KEEPASSX_ENTRYATTRIBUTESMODEL_H
#define KEEPASSX_ENTRYATTRIBUTESMODEL_H
#include <QAbstractListModel>
class EntryAttributes;

class EntryAttributesModel:public QAbstractListModel
{
	Q_OBJECT public:
	explicit EntryAttributesModel(
		QObject* parent = nullptr
	);
	void setEntryAttributes(
		EntryAttributes* entryAttributes
	);
	virtual int rowCount(
		const QModelIndex &parent = QModelIndex()
	) const override;
	virtual int columnCount(
		const QModelIndex &parent = QModelIndex()
	) const override;
	virtual QVariant headerData(
		int section,
		Qt::Orientation orientation,
		int role = Qt::DisplayRole
	) const override;
	virtual QVariant data(
		const QModelIndex &index,
		int role = Qt::DisplayRole
	) const override;
	virtual bool setData(
		const QModelIndex &index,
		const QVariant &value,
		int role = Qt::EditRole
	) override;
	virtual Qt::ItemFlags flags(
		const QModelIndex &index
	) const override;
	QModelIndex indexByKey(
		const QString &key
	) const;
	QString keyByIndex(
		const QModelIndex &index
	) const;
private Q_SLOTS:
	void do_attributeChange(
		const QString &key
	);
	void do_attributeAboutToAdd(
		const QString &key
	);
	void do_attributeAdd();
	void do_attributeAboutToRemove(
		const QString &key
	);
	void do_attributeRemove();
	void do_attributeAboutToRename(
		const QString &oldKey,
		const QString &newKey
	);
	void do_attributeRename(
		const QString &oldKey,
		const QString &newKey
	);
	void do_aboutToReset();
	void do_reset();
private:
	void updateAttributes();
	EntryAttributes* entryAttributes;
	QList<QString> attributes;
	bool nextRenameDataChange;
};
#endif // KEEPASSX_ENTRYATTRIBUTESMODEL_H
