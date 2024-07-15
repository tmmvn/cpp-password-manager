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
#ifndef KEEPASSX_SETTINGSWIDGET_H
#define KEEPASSX_SETTINGSWIDGET_H
#include "gui/EditWidget.h"

namespace Ui
{
	class SettingsWidgetGeneral;
	class SettingsWidgetSecurity;
}

class SettingsWidget final:public EditWidget
{
	Q_OBJECT public:
	explicit SettingsWidget(
		QWidget* parent = nullptr
	);
	virtual ~SettingsWidget() override;
	void loadSettings();
Q_SIGNALS:
	void sig_editFinished(
		bool accepted
	);
private Q_SLOTS:
	void do_saveSettings();
	void do_reject();
	void do_enableAutoSaveOnExit(
		bool checked
	) const;
private:
	QWidget* const secWidget;
	QWidget* const generalWidget;
	const QScopedPointer<Ui::SettingsWidgetSecurity> secUi;
	const QScopedPointer<Ui::SettingsWidgetGeneral> generalUi;
};
#endif // KEEPASSX_SETTINGSWIDGET_H
