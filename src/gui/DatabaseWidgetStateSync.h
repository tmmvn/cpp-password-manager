/*
 *  Copyright (C) 2014 Felix Geyer <debfx@fobos.de>
 *  Copyright (C) 2014 Florian Geyer <blueice@fobos.de>
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
#ifndef KEEPASSX_DATABASEWIDGETSTATESYNC_H
#define KEEPASSX_DATABASEWIDGETSTATESYNC_H
#include "gui/DatabaseWidget.h"

class DatabaseWidgetStateSync final:public QObject
{
	Q_OBJECT public:
	explicit DatabaseWidgetStateSync(
		QObject* parent = nullptr
	);
	virtual ~DatabaseWidgetStateSync() override;
public Q_SLOTS:
	void do_setActive(
		DatabaseWidget* dbWidget
	);
	void do_restoreListView();
	void do_restoreSearchView();
private Q_SLOTS:
	void do_getBlockUpdates();
	void do_updateSplitterSizes();
	void do_updateColumnSizes();
private:
	static QList<int> variantToIntList(
		const QVariant &variant
	);
	static QVariant intListToVariant(
		const QList<int> &list
	);
	DatabaseWidget* activeDbWidget;
	bool blockUpdates;
	QList<int> splitterSizes;
	QList<int> columnSizesList;
	QList<int> columnSizesSearch;
};
#endif // KEEPASSX_DATABASEWIDGETSTATESYNC_H
