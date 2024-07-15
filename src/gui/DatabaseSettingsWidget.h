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
#ifndef KEEPASSX_DATABASESETTINGSWIDGET_H
#define KEEPASSX_DATABASESETTINGSWIDGET_H
#include <QScopedPointer>
#include "gui/DialogWidget.h"
class Database;

namespace Ui
{
	class DatabaseSettingsWidget;
}

class DatabaseSettingsWidget:public DialogWidget
{
	Q_OBJECT public:
	explicit DatabaseSettingsWidget(
		QWidget* parent = nullptr
	);
	virtual ~DatabaseSettingsWidget() override;
	void load(
		Database* db
	);
Q_SIGNALS:
	void sig_editFinished(
		bool accepted
	);
private Q_SLOTS:
	void do_save();
	void do_reject();
	void do_transformRoundsBenchmark() const;
private:
	void truncateHistories() const;
	const QScopedPointer<Ui::DatabaseSettingsWidget> ui;
	Database* db;
	Q_DISABLE_COPY(
		DatabaseSettingsWidget
	)
};
#endif // KEEPASSX_DATABASESETTINGSWIDGET_H
