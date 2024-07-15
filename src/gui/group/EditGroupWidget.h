/*
 *  Copyright (C) 2011 Felix Geyer <debfx@fobos.de>
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
#ifndef KEEPASSX_EDITGROUPWIDGET_H
#define KEEPASSX_EDITGROUPWIDGET_H
#include <QComboBox>
#include <QScopedPointer>
#include "core/Group.h"
#include "gui/EditWidget.h"
class EditWidgetIcons;
class EditWidgetProperties;

namespace Ui
{
	class EditGroupWidgetMain;
	class EditWidget;
}

class EditGroupWidget final:public EditWidget
{
	Q_OBJECT public:
	explicit EditGroupWidget(
		QWidget* parent = nullptr
	);
	virtual ~EditGroupWidget() override;
	void loadGroup(
		Group* group,
		bool create,
		Database* database
	);
	void clear();
Q_SIGNALS:
	void sig_editFinished(
		bool accepted
	);
private Q_SLOTS:
	void do_save();
	void do_cancel();
private:
	static void addTriStateItems(
		QComboBox* comboBox,
		bool inheritValue
	);
	static int indexFromTriState(
		Group::TriState triState
	);
	static Group::TriState triStateFromIndex(
		int index
	);
	const QScopedPointer<Ui::EditGroupWidgetMain> mainUi;
	QWidget* const editGroupWidgetMain;
	EditWidgetIcons* const editGroupWidgetIcons;
	EditWidgetProperties* const editWidgetProperties;
	Group* group;
	Database* database;
	Q_DISABLE_COPY(
		EditGroupWidget
	)
};
#endif // KEEPASSX_EDITGROUPWIDGET_H
