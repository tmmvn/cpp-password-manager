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
#ifndef KEEPASSX_MAINWINDOW_H
#define KEEPASSX_MAINWINDOW_H
#include <QActionGroup>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include "gui/DatabaseWidget.h"

namespace Ui
{
	class MainWindow;
}

class InactivityTimer;

class MainWindow final:public QMainWindow
{
	Q_OBJECT public:
	MainWindow();
	virtual ~MainWindow() override;
	void setCurrentDatabaseWidget(
		DatabaseWidget* widget
	);
	void openDatabase(
		const QString &fileName,
		const QString &pw = QString(),
		const QString &keyFile = QString()
	) const;
public Q_SLOTS:
	void do_openDatabase(
		const QString &fileName
	) const;
protected:
	virtual void closeEvent(
		QCloseEvent* event
	) override;
	virtual void changeEvent(
		QEvent* event
	) override;
private Q_SLOTS:
	void do_setMenuActionState() const;
	void do_updateWindowTitle();
	void do_showAboutDialog();
	void do_switchToDatabases() const;
	void do_switchToSettings() const;
	void do_databaseTabChanged(
		int tabIndex
	);
	void do_openRecentDatabase(
		const QAction* action
	) const;
	static void do_clearLastDatabases();
	void do_updateLastDatabasesMenu() const;
	void do_updateCopyAttributesMenu() const;
	void do_showEntryContextMenu(
		const QPoint &globalPos
	) const;
	void do_showGroupContextMenu(
		const QPoint &globalPos
	) const;
	static void do_saveToolbarState(
		bool value
	);
	void do_rememberOpenDatabases(
		const QString &filePath
	);
	void do_applySettingsChanges();
	void do_trayIconTriggered(
		QSystemTrayIcon::ActivationReason reason
	);
	void do_toggleWindow();
	void do_lockDatabasesAfterInactivity() const;
	void do_repairDatabase();
	void do_createEntry() const;
	void do_cloneEntry() const;
	void do_switchToEntryEdit() const;
	void do_deleteEntries() const;
	void do_copyTitle() const;
	void do_copyUsername() const;
	void do_copyPassword() const;
	void do_copyURL() const;
	void do_copyNotes() const;
	void do_copyAttribute(
		const QAction* action
	) const;
	void do_performAutoType() const;
	void do_openUrl() const;
	void do_createGroup() const;
	void do_switchToGroupEdit() const;
	void do_deleteGroup() const;
	void do_openSearch() const;
private:
	static void setShortcut(
		QAction* action,
		QKeySequence::StandardKey standard,
		QKeyCombination fallback
	);
	static const QString BaseWindowTitle;
	void saveWindowInformation() const;
	bool saveLastDatabases();
	void updateTrayIcon();
	static bool isTrayIconEnabled();
	void copyAttribute(
		const QAction* action
	) const;
	const QScopedPointer<Ui::MainWindow> ui;
	QAction* clearHistoryAction;
	QActionGroup* lastDatabasesActions;
	QActionGroup* copyAdditionalAttributeActions;
	QStringList openDatabases;
	InactivityTimer* inactivityTimer;
	int countDefaultAttributes;
	QSystemTrayIcon* trayIcon;
	DatabaseWidget* currentDatabaseWidget;
	Q_DISABLE_COPY(
		MainWindow
	)
};
#endif // KEEPASSX_MAINWINDOW_H
