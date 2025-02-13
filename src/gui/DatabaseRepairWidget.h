/*
 *  Copyright (C) 2016 Felix Geyer <debfx@fobos.de>
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
#ifndef KEEPASSX_DATABASEREPAIRWIDGET_H
#define KEEPASSX_DATABASEREPAIRWIDGET_H
#include "gui/DatabaseOpenWidget.h"

class DatabaseRepairWidget final:public DatabaseOpenWidget
{
	Q_OBJECT public:
	explicit DatabaseRepairWidget(
		QWidget* parent = nullptr
	);
Q_SIGNALS:
	void sig_success();
	void sig_error();
protected:
	virtual void do_openDatabase() override;
private Q_SLOTS:
	void do_processEditFinished(
		bool result
	);
};
#endif // KEEPASSX_DATABASEREPAIRWIDGET_H
