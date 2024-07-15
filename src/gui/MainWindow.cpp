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
#include "MainWindow.h"
#include <QCloseEvent>
#include <QShortcut>
#include <QTimer>
#include "ui_MainWindow.h"
#include "core/Config.h"
#include "core/FilePath.h"
#include "core/InactivityTimer.h"
#include "format/KeePass2Writer.h"
#include "gui/AboutDialog.h"
#include "gui/DatabaseRepairWidget.h"
#include "gui/DatabaseWidget.h"
#include "gui/FileDialog.h"
#include "gui/MessageBox.h"
const QString MainWindow::BaseWindowTitle = "KeePassX";

MainWindow::MainWindow()
	: ui(
		new Ui::MainWindow()
	),
	trayIcon(
		nullptr
	)
{
	this->ui->setupUi(
		this
	);
	this->countDefaultAttributes = static_cast<int>(this->ui->
		menuEntryCopyAttribute->actions().size());
	this->restoreGeometry(
		Config::getInstance()->get(
			"GUI/MainWindowGeometry"
		).toByteArray()
	);
	this->setWindowIcon(
		FilePath::getInstance()->getApplicationIcon()
	);
	QAction* toggleViewAction_ = this->ui->toolBar->toggleViewAction();
	toggleViewAction_->setText(
		this->tr(
			"Show toolbar"
		)
	);
	this->ui->menuView->addAction(
		toggleViewAction_
	);
	const bool showToolbar_ = Config::getInstance()->get(
		"ShowToolbar"
	).toBool();
	this->ui->toolBar->setVisible(
		showToolbar_
	);
	this->connect(
		this->ui->toolBar,
		&QToolBar::visibilityChanged,
		this,
		&MainWindow::do_saveToolbarState
	);
	this->clearHistoryAction = new QAction(
		"Clear history",
		ui->menuFile
	);
	this->lastDatabasesActions = new QActionGroup(
		this->ui->menuRecentDatabases
	);
	this->connect(
		this->clearHistoryAction,
		&QAction::triggered,
		this,
		&MainWindow::do_clearLastDatabases
	);
	this->connect(
		this->lastDatabasesActions,
		&QActionGroup::triggered,
		this,
		&MainWindow::do_openRecentDatabase
	);
	this->connect(
		this->ui->menuRecentDatabases,
		&QMenu::aboutToShow,
		this,
		&MainWindow::do_updateLastDatabasesMenu
	);
	this->copyAdditionalAttributeActions = new QActionGroup(
		this->ui->menuEntryCopyAttribute
	);
	this->connect(
		this->copyAdditionalAttributeActions,
		&QActionGroup::triggered,
		this,
		&MainWindow::do_copyAttribute
	);
	this->connect(
		this->ui->menuEntryCopyAttribute,
		&QMenu::aboutToShow,
		this,
		&MainWindow::do_updateCopyAttributesMenu
	);
	this->inactivityTimer = new InactivityTimer(
		this
	);
	this->connect(
		this->inactivityTimer,
		&InactivityTimer::sig_inactivityDetected,
		this,
		&MainWindow::do_lockDatabasesAfterInactivity
	);
	this->do_applySettingsChanges();
	this->setShortcut(
		this->ui->actionDatabaseOpen,
		QKeySequence::Open,
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_O
		)
	);
	this->setShortcut(
		this->ui->actionDatabaseSave,
		QKeySequence::Save,
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_S
		)
	);
	this->setShortcut(
		this->ui->actionDatabaseSaveAs,
		QKeySequence::SaveAs,
		QKeyCombination(
			Qt::CTRL | Qt::SHIFT,
			Qt::Key_S
		)
	);
	this->setShortcut(
		this->ui->actionDatabaseClose,
		QKeySequence::Close,
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_W
		)
	);
	this->ui->actionLockDatabases->setShortcut(
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_L
		)
	);
	this->setShortcut(
		this->ui->actionQuit,
		QKeySequence::Quit,
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_Q
		)
	);
	this->setShortcut(
		this->ui->actionSearch,
		QKeySequence::Find,
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_F
		)
	);
	this->ui->actionEntryNew->setShortcut(
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_N
		)
	);
	this->ui->actionEntryEdit->setShortcut(
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_E
		)
	);
	this->ui->actionEntryDelete->setShortcut(
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_D
		)
	);
	this->ui->actionEntryClone->setShortcut(
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_K
		)
	);
	this->ui->actionEntryCopyUsername->setShortcut(
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_B
		)
	);
	this->ui->actionEntryCopyPassword->setShortcut(
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_C
		)
	);
	this->ui->actionEntryOpenUrl->setShortcut(
		QKeyCombination(
			Qt::CTRL,
			Qt::Key_U
		)
	);
	this->ui->actionEntryCopyURL->setShortcut(
		QKeyCombination(
			Qt::CTRL | Qt::ALT,
			Qt::Key_U
		)
	);
	// TODO: Figure out how to not use SLOT macro with this
	/*new QShortcut(
		Qt::CTRL | Qt::Key_M,
		this,
		&MainWindow::showMinimized
	);*/
	this->ui->actionDatabaseNew->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"document-new"
		)
	);
	this->ui->actionDatabaseOpen->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"document-open"
		)
	);
	this->ui->actionDatabaseSave->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"document-save"
		)
	);
	this->ui->actionDatabaseSaveAs->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"document-save-as"
		)
	);
	this->ui->actionDatabaseClose->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"document-close"
		)
	);
	this->ui->actionChangeDatabaseSettings->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"document-edit"
		)
	);
	this->ui->actionChangeMasterKey->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"database-change-key",
			false
		)
	);
	this->ui->actionLockDatabases->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"document-encrypt",
			false
		)
	);
	this->ui->actionQuit->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"application-exit"
		)
	);
	this->ui->actionEntryNew->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"entry-new",
			false
		)
	);
	this->ui->actionEntryClone->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"entry-clone",
			false
		)
	);
	this->ui->actionEntryEdit->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"entry-edit",
			false
		)
	);
	this->ui->actionEntryDelete->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"entry-delete",
			false
		)
	);
	this->ui->actionEntryCopyUsername->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"username-copy",
			false
		)
	);
	this->ui->actionEntryCopyPassword->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"password-copy",
			false
		)
	);
	this->ui->actionGroupNew->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"group-new",
			false
		)
	);
	this->ui->actionGroupEdit->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"group-edit",
			false
		)
	);
	this->ui->actionGroupDelete->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"group-delete",
			false
		)
	);
	this->ui->actionSettings->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"configure"
		)
	);
	this->ui->actionAbout->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"help-about"
		)
	);
	this->ui->actionSearch->setIcon(
		FilePath::getInstance()->getIcon(
			"actions",
			"system-search"
		)
	);
	// Connect signals directly to the current database widget
	this->connect(
		this->ui->tabWidget,
		&DatabaseTabWidget::sig_activateDatabaseChanged,
		this,
		&MainWindow::setCurrentDatabaseWidget
	);
	this->connect(
		this->ui->tabWidget,
		&DatabaseTabWidget::sig_tabNameChanged,
		this,
		&MainWindow::do_updateWindowTitle
	);
	this->connect(
		this->ui->tabWidget,
		&DatabaseTabWidget::currentChanged,
		this,
		&MainWindow::do_updateWindowTitle
	);
	this->connect(
		this->ui->tabWidget,
		&DatabaseTabWidget::currentChanged,
		this,
		&MainWindow::do_databaseTabChanged
	);
	this->connect(
		this->ui->stackedWidget,
		&QStackedWidget::currentChanged,
		this,
		&MainWindow::do_setMenuActionState
	);
	this->connect(
		this->ui->stackedWidget,
		&QStackedWidget::currentChanged,
		this,
		&MainWindow::do_updateWindowTitle
	);
	this->connect(
		this->ui->settingsWidget,
		&SettingsWidget::sig_editFinished,
		this,
		&MainWindow::do_switchToDatabases
	);
	this->connect(
		this->ui->settingsWidget,
		&SettingsWidget::sig_accepted,
		this,
		&MainWindow::do_applySettingsChanges
	);
	this->connect(
		this->ui->actionDatabaseNew,
		&QAction::triggered,
		this->ui->tabWidget,
		&DatabaseTabWidget::do_newDatabase
	);
	this->connect(
		this->ui->actionDatabaseOpen,
		&QAction::triggered,
		this->ui->tabWidget,
		&DatabaseTabWidget::do_openDatabase
	);
	this->connect(
		this->ui->actionDatabaseSave,
		&QAction::triggered,
		this->ui->tabWidget,
		&DatabaseTabWidget::do_saveDatabase
	);
	this->connect(
		this->ui->actionDatabaseSaveAs,
		&QAction::triggered,
		this->ui->tabWidget,
		&DatabaseTabWidget::do_saveDatabaseAs
	);
	this->connect(
		this->ui->actionDatabaseClose,
		&QAction::triggered,
		this->ui->tabWidget,
		&DatabaseTabWidget::do_closeDatabase
	);
	this->connect(
		this->ui->actionChangeMasterKey,
		&QAction::triggered,
		this->ui->tabWidget,
		&DatabaseTabWidget::do_changeMasterKey
	);
	this->connect(
		this->ui->actionChangeDatabaseSettings,
		&QAction::triggered,
		this->ui->tabWidget,
		&DatabaseTabWidget::do_changeDatabaseSettings
	);
	this->connect(
		this->ui->actionRepairDatabase,
		&QAction::triggered,
		this,
		&MainWindow::do_repairDatabase
	);
	this->connect(
		this->ui->actionExportCsv,
		&QAction::triggered,
		this->ui->tabWidget,
		&DatabaseTabWidget::do_exportToCsv
	);
	this->connect(
		this->ui->actionLockDatabases,
		&QAction::triggered,
		this->ui->tabWidget,
		&DatabaseTabWidget::do_lockDatabases
	);
	this->connect(
		this->ui->actionQuit,
		&QAction::triggered,
		this,
		&MainWindow::close
	);
	this->connect(
		this->ui->actionEntryNew,
		&QAction::triggered,
		this,
		&MainWindow::do_createEntry
	);
	this->connect(
		this->ui->actionEntryClone,
		&QAction::triggered,
		this,
		&MainWindow::do_cloneEntry
	);
	this->connect(
		this->ui->actionEntryEdit,
		&QAction::triggered,
		this,
		&MainWindow::do_switchToEntryEdit
	);
	this->connect(
		this->ui->actionEntryDelete,
		&QAction::triggered,
		this,
		&MainWindow::do_deleteEntries
	);
	this->connect(
		this->ui->actionEntryCopyTitle,
		&QAction::triggered,
		this,
		&MainWindow::do_copyTitle
	);
	this->connect(
		this->ui->actionEntryCopyUsername,
		&QAction::triggered,
		this,
		&MainWindow::do_copyUsername
	);
	this->connect(
		this->ui->actionEntryCopyPassword,
		&QAction::triggered,
		this,
		&MainWindow::do_copyPassword
	);
	this->connect(
		this->ui->actionEntryCopyURL,
		&QAction::triggered,
		this,
		&MainWindow::do_copyURL
	);
	this->connect(
		this->ui->actionEntryCopyNotes,
		&QAction::triggered,
		this,
		&MainWindow::do_copyNotes
	);
	this->connect(
		this->ui->actionEntryOpenUrl,
		&QAction::triggered,
		this,
		&MainWindow::do_openUrl
	);
	this->connect(
		this->ui->actionGroupNew,
		&QAction::triggered,
		this,
		&MainWindow::do_createGroup
	);
	this->connect(
		this->ui->actionGroupEdit,
		&QAction::triggered,
		this,
		&MainWindow::do_switchToGroupEdit
	);
	this->connect(
		this->ui->actionGroupDelete,
		&QAction::triggered,
		this,
		&MainWindow::do_deleteGroup
	);
	this->connect(
		this->ui->actionSettings,
		&QAction::triggered,
		this,
		&MainWindow::do_switchToSettings
	);
	this->connect(
		this->ui->actionAbout,
		&QAction::triggered,
		this,
		&MainWindow::do_showAboutDialog
	);
	this->connect(
		this->ui->actionSearch,
		&QAction::triggered,
		this,
		&MainWindow::do_openSearch
	);
	// Initially set menu action state
	this->do_setMenuActionState();
}

MainWindow::~MainWindow()
{
}

void MainWindow::openDatabase(
	const QString &fileName,
	const QString &pw,
	const QString &keyFile
) const
{
	this->ui->tabWidget->openDatabase(
		fileName,
		pw,
		keyFile
	);
}

void MainWindow::setCurrentDatabaseWidget(
	DatabaseWidget* widget
)
{
	if(this->currentDatabaseWidget)
	{
		this->disconnect(
			this->currentDatabaseWidget,
			&DatabaseWidget::sig_currentModeChanged,
			this,
			&MainWindow::do_setMenuActionState
		);
		this->disconnect(
			this->currentDatabaseWidget,
			&DatabaseWidget::sig_groupChanged,
			this,
			&MainWindow::do_setMenuActionState
		);
		this->disconnect(
			this->currentDatabaseWidget,
			&DatabaseWidget::sig_entrySelectionChanged,
			this,
			&MainWindow::do_setMenuActionState
		);
		this->disconnect(
			this->currentDatabaseWidget,
			&DatabaseWidget::sig_groupContextMenuRequested,
			this,
			&MainWindow::do_showGroupContextMenu
		);
		this->disconnect(
			this->currentDatabaseWidget,
			&DatabaseWidget::sig_entryContextMenuRequested,
			this,
			&MainWindow::do_showEntryContextMenu
		);
	}
	this->currentDatabaseWidget = widget;
	if(this->currentDatabaseWidget)
	{
		this->connect(
			this->currentDatabaseWidget,
			&DatabaseWidget::sig_currentModeChanged,
			this,
			&MainWindow::do_setMenuActionState
		);
		this->connect(
			this->currentDatabaseWidget,
			&DatabaseWidget::sig_groupChanged,
			this,
			&MainWindow::do_setMenuActionState
		);
		this->connect(
			this->currentDatabaseWidget,
			&DatabaseWidget::sig_entrySelectionChanged,
			this,
			&MainWindow::do_setMenuActionState
		);
		this->connect(
			this->currentDatabaseWidget,
			&DatabaseWidget::sig_groupContextMenuRequested,
			this,
			&MainWindow::do_showGroupContextMenu
		);
		this->connect(
			this->currentDatabaseWidget,
			&DatabaseWidget::sig_entryContextMenuRequested,
			this,
			&MainWindow::do_showEntryContextMenu
		);
	}
	this->do_setMenuActionState();
}

void MainWindow::do_updateLastDatabasesMenu() const
{
	this->ui->menuRecentDatabases->clear();
	const QStringList lastDatabases_ = Config::getInstance()->get(
		"LastDatabases",
		QVariant()
	).toStringList();
	for(const QString &database_: lastDatabases_)
	{
		QAction* action_ = this->ui->menuRecentDatabases->addAction(
			database_
		);
		action_->setData(
			database_
		);
		this->lastDatabasesActions->addAction(
			action_
		);
	}
	this->ui->menuRecentDatabases->addSeparator();
	this->ui->menuRecentDatabases->addAction(
		this->clearHistoryAction
	);
}

void MainWindow::do_updateCopyAttributesMenu() const
{
	const DatabaseWidget* dbWidget_ = this->ui->tabWidget->
		getCurrentDatabaseWidget();
	if(!dbWidget_)
	{
		return;
	}
	if(dbWidget_->getNumberOfSelectedEntries() != 1)
	{
		return;
	}
	QList<QAction*> actions_ = this->ui->menuEntryCopyAttribute->actions();
	for(int i_ = this->countDefaultAttributes; i_ < actions_.size(); i_++)
	{
		delete actions_[i_];
	}
	const QStringList customEntryAttributes_ = dbWidget_->
		getCustomEntryAttributes();
	for(const QString &key_: customEntryAttributes_)
	{
		QAction* action_ = this->ui->menuEntryCopyAttribute->addAction(
			key_
		);
		this->copyAdditionalAttributeActions->addAction(
			action_
		);
	}
}

void MainWindow::do_openRecentDatabase(
	const QAction* action
) const
{
	this->do_openDatabase(
		action->data().toString()
	);
}

void MainWindow::do_clearLastDatabases()
{
	Config::getInstance()->set(
		"LastDatabases",
		QVariant()
	);
}

void MainWindow::do_openDatabase(
	const QString &fileName
) const
{
	const auto &pw = QString();
	const auto &keyFile = QString();
	this->ui->tabWidget->openDatabase(
		fileName,
		pw,
		keyFile
	);
}

void MainWindow::do_setMenuActionState() const
{
	const bool inDatabaseTabWidget_ = this->ui->stackedWidget->currentIndex() ==
		0;
	const bool inWelcomeWidget_ = this->ui->stackedWidget->currentIndex() == 2;
	if(inDatabaseTabWidget_ && this->ui->tabWidget->currentIndex() != -1)
	{
		switch(const DatabaseWidget* dbWidget_ = this->ui->tabWidget->
			getCurrentDatabaseWidget(); dbWidget_->getCurrentMode())
		{
			case DatabaseWidget::ViewMode:
			{
				const bool inSearch_ = dbWidget_->isInSearchMode();
				const bool singleEntrySelected_ = dbWidget_->
					getNumberOfSelectedEntries() == 1;
				const bool entriesSelected_ = dbWidget_->
					getNumberOfSelectedEntries() > 0;
				const bool groupSelected_ = dbWidget_->isGroupSelected();
				this->ui->actionEntryNew->setEnabled(
					!inSearch_
				);
				this->ui->actionEntryClone->setEnabled(
					singleEntrySelected_ && !inSearch_
				);
				this->ui->actionEntryEdit->setEnabled(
					singleEntrySelected_
				);
				this->ui->actionEntryDelete->setEnabled(
					entriesSelected_
				);
				this->ui->actionEntryCopyTitle->setEnabled(
					singleEntrySelected_ && dbWidget_->currentEntryHasTitle()
				);
				this->ui->actionEntryCopyUsername->setEnabled(
					singleEntrySelected_ && dbWidget_->currentEntryHasUsername()
				);
				this->ui->actionEntryCopyPassword->setEnabled(
					singleEntrySelected_ && dbWidget_->currentEntryHasPassword()
				);
				this->ui->actionEntryCopyURL->setEnabled(
					singleEntrySelected_ && dbWidget_->currentEntryHasUrl()
				);
				this->ui->actionEntryCopyNotes->setEnabled(
					singleEntrySelected_ && dbWidget_->currentEntryHasNotes()
				);
				this->ui->menuEntryCopyAttribute->setEnabled(
					singleEntrySelected_
				);
				this->ui->actionEntryOpenUrl->setEnabled(
					singleEntrySelected_ && dbWidget_->currentEntryHasUrl()
				);
				this->ui->actionGroupNew->setEnabled(
					groupSelected_
				);
				this->ui->actionGroupEdit->setEnabled(
					groupSelected_
				);
				this->ui->actionGroupDelete->setEnabled(
					groupSelected_ && dbWidget_->canDeleteCurrentGroup()
				);
				// TODO: get checked state from db widget
				this->ui->actionSearch->setEnabled(
					true
				);
				this->ui->actionChangeMasterKey->setEnabled(
					true
				);
				this->ui->actionChangeDatabaseSettings->setEnabled(
					true
				);
				this->ui->actionDatabaseSave->setEnabled(
					true
				);
				this->ui->actionDatabaseSaveAs->setEnabled(
					true
				);
				this->ui->actionExportCsv->setEnabled(
					true
				);
				break;
			}
			case DatabaseWidget::EditMode:
			case DatabaseWidget::LockedMode:
			{
				const QList<QAction*> entryActions_ = this->ui->menuEntries->
					actions();
				for(QAction* action_: entryActions_)
				{
					action_->setEnabled(
						false
					);
				}
				const QList<QAction*> groupActions_ = this->ui->menuGroups->
					actions();
				for(QAction* action_: groupActions_)
				{
					action_->setEnabled(
						false
					);
				}
				this->ui->actionEntryCopyTitle->setEnabled(
					false
				);
				this->ui->actionEntryCopyUsername->setEnabled(
					false
				);
				this->ui->actionEntryCopyPassword->setEnabled(
					false
				);
				this->ui->actionEntryCopyURL->setEnabled(
					false
				);
				this->ui->actionEntryCopyNotes->setEnabled(
					false
				);
				this->ui->menuEntryCopyAttribute->setEnabled(
					false
				);
				this->ui->actionSearch->setEnabled(
					false
				);
				this->ui->actionChangeMasterKey->setEnabled(
					false
				);
				this->ui->actionChangeDatabaseSettings->setEnabled(
					false
				);
				this->ui->actionDatabaseSave->setEnabled(
					false
				);
				this->ui->actionDatabaseSaveAs->setEnabled(
					false
				);
				this->ui->actionExportCsv->setEnabled(
					false
				);
				break;
			}
			default:
				break;
		}
		this->ui->actionDatabaseClose->setEnabled(
			true
		);
	}
	else
	{
		const QList<QAction*> entryActions_ = this->ui->menuEntries->actions();
		for(QAction* action_: entryActions_)
		{
			action_->setEnabled(
				false
			);
		}
		const QList<QAction*> groupActions_ = this->ui->menuGroups->actions();
		for(QAction* action_: groupActions_)
		{
			action_->setEnabled(
				false
			);
		}
		this->ui->actionEntryCopyTitle->setEnabled(
			false
		);
		this->ui->actionEntryCopyUsername->setEnabled(
			false
		);
		this->ui->actionEntryCopyPassword->setEnabled(
			false
		);
		this->ui->actionEntryCopyURL->setEnabled(
			false
		);
		this->ui->actionEntryCopyNotes->setEnabled(
			false
		);
		this->ui->menuEntryCopyAttribute->setEnabled(
			false
		);
		this->ui->actionSearch->setEnabled(
			false
		);
		this->ui->actionChangeMasterKey->setEnabled(
			false
		);
		this->ui->actionChangeDatabaseSettings->setEnabled(
			false
		);
		this->ui->actionDatabaseSave->setEnabled(
			false
		);
		this->ui->actionDatabaseSaveAs->setEnabled(
			false
		);
		this->ui->actionDatabaseClose->setEnabled(
			false
		);
		this->ui->actionExportCsv->setEnabled(
			false
		);
	}
	const bool inDatabaseTabWidgetOrWelcomeWidget_ = inDatabaseTabWidget_ ||
		inWelcomeWidget_;
	this->ui->actionDatabaseNew->setEnabled(
		inDatabaseTabWidgetOrWelcomeWidget_
	);
	this->ui->actionDatabaseOpen->setEnabled(
		inDatabaseTabWidgetOrWelcomeWidget_
	);
	this->ui->menuRecentDatabases->setEnabled(
		inDatabaseTabWidgetOrWelcomeWidget_
	);
	this->ui->actionRepairDatabase->setEnabled(
		inDatabaseTabWidgetOrWelcomeWidget_
	);
	this->ui->actionLockDatabases->setEnabled(
		this->ui->tabWidget->hasLockableDatabases()
	);
}

void MainWindow::do_updateWindowTitle()
{
	QString customWindowTitlePart_;
	const int stackedWidgetIndex_ = this->ui->stackedWidget->currentIndex();
	if(const int tabWidgetIndex_ = this->ui->tabWidget->currentIndex();
		stackedWidgetIndex_ == 0 && tabWidgetIndex_ != -1)
	{
		customWindowTitlePart_ = this->ui->tabWidget->tabText(
			tabWidgetIndex_
		);
		if(this->ui->tabWidget->do_readOnly(
			tabWidgetIndex_
		))
		{
			customWindowTitlePart_.append(
				QString(
					" [%1]"
				).arg(
					this->tr(
						"read-only"
					)
				)
			);
		}
	}
	else if(stackedWidgetIndex_ == 1)
	{
		customWindowTitlePart_ = tr(
			"Settings"
		);
	}
	QString windowTitle_;
	if(customWindowTitlePart_.isEmpty())
	{
		windowTitle_ = this->BaseWindowTitle;
	}
	else
	{
		windowTitle_ = QString(
			"%1 - %2"
		).arg(
			customWindowTitlePart_,
			BaseWindowTitle
		);
	}
	this->setWindowTitle(
		windowTitle_
	);
}

void MainWindow::do_showAboutDialog()
{
	const auto aboutDialog_ = new AboutDialog(
		this
	);
	aboutDialog_->show();
}

void MainWindow::do_switchToDatabases() const
{
	if(this->ui->tabWidget->currentIndex() == -1)
	{
		this->ui->stackedWidget->setCurrentIndex(
			2
		);
	}
	else
	{
		this->ui->stackedWidget->setCurrentIndex(
			0
		);
	}
}

void MainWindow::do_switchToSettings() const
{
	this->ui->settingsWidget->loadSettings();
	this->ui->stackedWidget->setCurrentIndex(
		1
	);
}

void MainWindow::do_databaseTabChanged(
	const int tabIndex
)
{
	if(tabIndex != -1 && this->ui->stackedWidget->currentIndex() == 2)
	{
		this->ui->stackedWidget->setCurrentIndex(
			0
		);
	}
	else if(tabIndex == -1 && this->ui->stackedWidget->currentIndex() == 0)
	{
		this->ui->stackedWidget->setCurrentIndex(
			2
		);
	}
	// Update the current database widget
	this->setCurrentDatabaseWidget(
		this->ui->tabWidget->getCurrentDatabaseWidget()
	);
}

void MainWindow::closeEvent(
	QCloseEvent* event
)
{
	if(saveLastDatabases())
	{
		this->saveWindowInformation();
		event->accept();
		QApplication::quit();
	}
	else
	{
		event->ignore();
	}
}

void MainWindow::changeEvent(
	QEvent* event
)
{
	if((event->type() == QEvent::WindowStateChange) && this->isMinimized() &&
		this->isTrayIconEnabled() && this->trayIcon && this->trayIcon->
		isVisible() && Config::getInstance()->get(
			"GUI/MinimizeToTray"
		).toBool())
	{
		event->ignore();
		QTimer::singleShot(
			0,
			this,
			&MainWindow::hide
		);
	}
	else
	{
		QMainWindow::changeEvent(
			event
		);
	}
}

void MainWindow::saveWindowInformation() const
{
	Config::getInstance()->set(
		"GUI/MainWindowGeometry",
		this->saveGeometry()
	);
}

bool MainWindow::saveLastDatabases()
{
	bool accept_;
	this->openDatabases.clear();
	const bool openPreviousDatabasesOnStartup_ = Config::getInstance()->get(
		"OpenPreviousDatabasesOnStartup"
	).toBool();
	if(openPreviousDatabasesOnStartup_)
	{
		this->connect(
			this->ui->tabWidget,
			&DatabaseTabWidget::sig_databaseWithFileClosed,
			this,
			&MainWindow::do_rememberOpenDatabases
		);
	}
	if(!this->ui->tabWidget->do_closeAllDatabases())
	{
		accept_ = false;
	}
	else
	{
		accept_ = true;
	}
	if(openPreviousDatabasesOnStartup_)
	{
		this->disconnect(
			this->ui->tabWidget,
			&DatabaseTabWidget::sig_databaseWithFileClosed,
			this,
			&MainWindow::do_rememberOpenDatabases
		);
		Config::getInstance()->set(
			"LastOpenedDatabases",
			this->openDatabases
		);
	}
	return accept_;
}

void MainWindow::updateTrayIcon()
{
	if(this->isTrayIconEnabled())
	{
		if(!this->trayIcon)
		{
			this->trayIcon = new QSystemTrayIcon(
				FilePath::getInstance()->getApplicationIcon(),
				this
			);
			const auto menu_ = new QMenu(
				this
			);
			const auto actionToggle_ = new QAction(
				tr(
					"Toggle window"
				),
				menu_
			);
			menu_->addAction(
				actionToggle_
			);
			menu_->addAction(
				this->ui->actionQuit
			);
			this->connect(
				this->trayIcon,
				&QSystemTrayIcon::activated,
				this,
				&MainWindow::do_trayIconTriggered
			);
			this->connect(
				actionToggle_,
				&QAction::triggered,
				this,
				&MainWindow::do_toggleWindow
			);
			this->trayIcon->setContextMenu(
				menu_
			);
			this->trayIcon->show();
		}
	}
	else
	{
		if(this->trayIcon)
		{
			this->trayIcon->hide();
			delete this->trayIcon;
			this->trayIcon = nullptr;
		}
	}
}

void MainWindow::do_showEntryContextMenu(
	const QPoint &globalPos
) const
{
	this->ui->menuEntries->popup(
		globalPos
	);
}

void MainWindow::do_showGroupContextMenu(
	const QPoint &globalPos
) const
{
	this->ui->menuGroups->popup(
		globalPos
	);
}

void MainWindow::do_saveToolbarState(
	const bool value
)
{
	Config::getInstance()->set(
		"ShowToolbar",
		value
	);
}

void MainWindow::setShortcut(
	QAction* action,
	const QKeySequence::StandardKey standard,
	const QKeyCombination fallback
)
{
	if(!QKeySequence::keyBindings(
		standard
	).isEmpty())
	{
		action->setShortcuts(
			standard
		);
	}
	action->setShortcut(
		fallback
	);
}

void MainWindow::do_rememberOpenDatabases(
	const QString &filePath
)
{
	this->openDatabases.append(
		filePath
	);
}

void MainWindow::do_applySettingsChanges()
{
	int timeout = Config::getInstance()->get(
		"security/lockdatabaseidlesec"
	).toInt() * 1000;
	if(timeout <= 0)
	{
		timeout = 60;
	}
	this->inactivityTimer->setInactivityTimeout(
		timeout
	);
	if(Config::getInstance()->get(
		"security/lockdatabaseidle"
	).toBool())
	{
		this->inactivityTimer->activate();
	}
	else
	{
		this->inactivityTimer->deactivate();
	}
	this->updateTrayIcon();
}

void MainWindow::do_trayIconTriggered(
	const QSystemTrayIcon::ActivationReason reason
)
{
	if(reason == QSystemTrayIcon::Trigger)
	{
		this->do_toggleWindow();
	}
}

void MainWindow::do_toggleWindow()
{
	if((QApplication::activeWindow() == this) && this->isVisible() && !this->
		isMinimized())
	{
		this->hide();
	}
	else
	{
		this->ensurePolished();
		this->setWindowState(
			this->windowState() & ~Qt::WindowMinimized
		);
		this->show();
		this->raise();
		this->activateWindow();
	}
}

void MainWindow::do_lockDatabasesAfterInactivity() const
{
	// ignore event if a modal dialog is open (such as a message box or file dialog)
	if(QApplication::activeModalWidget())
	{
		return;
	}
	this->ui->tabWidget->do_lockDatabases();
}

void MainWindow::do_repairDatabase()
{
	const QString filter_ = QString(
		"%1 (*.kdbx);;%2 (*)"
	).arg(
		this->tr(
			"KeePass 2 Database"
		),
		this->tr(
			"All files"
		)
	);
	const QString fileName_ = FileDialog::getInstance()->getOpenFileName(
		this,
		this->tr(
			"Open database"
		),
		QString(),
		filter_
	);
	if(fileName_.isEmpty())
	{
		return;
	}
	const QScopedPointer dialog_(
		new QDialog(
			this
		)
	);
	const auto dbRepairWidget_ = new DatabaseRepairWidget(
		dialog_.data()
	);
	this->connect(
		dbRepairWidget_,
		&DatabaseRepairWidget::sig_success,
		dialog_.data(),
		&QDialog::accept
	);
	this->connect(
		dbRepairWidget_,
		&DatabaseRepairWidget::sig_error,
		dialog_.data(),
		&QDialog::reject
	);
	dbRepairWidget_->load(
		fileName_
	);
	Database* db_ = dbRepairWidget_->database();
	if(dialog_->exec() == QDialog::Accepted && db_ != nullptr)
	{
		if(const QString saveFileName_ = FileDialog::getInstance()->
				getSaveFileName(
					this,
					this->tr(
						"Save repaired database"
					),
					QString(),
					this->tr(
						"KeePass 2 Database"
					).append(
						" (*.kdbx)"
					),
					nullptr,
					"kdbx"
				);
			!saveFileName_.isEmpty())
		{
			KeePass2Writer writer_;
			writer_.writeDatabase(
				saveFileName_,
				db_
			);
			if(writer_.hasError())
			{
				MessageBox::critical(
					this,
					this->tr(
						"Error"
					),
					this->tr(
						"Writing the database failed."
					) + "\n\n" + writer_.getErrorString()
				);
			}
			delete db_;
		}
	}
}

bool MainWindow::isTrayIconEnabled()
{
#ifdef Q_OS_MAC
	// systray not useful on OS X
	return false;
#else
    return Config::getInstance()->get("GUI/ShowTrayIcon").toBool()
            && QSystemTrayIcon::isSystemTrayAvailable();
#endif
}

// Add new slots for handling actions without signal multiplexer
void MainWindow::do_createEntry() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_createEntry();
	}
}

void MainWindow::do_cloneEntry() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_cloneEntry();
	}
}

void MainWindow::do_switchToEntryEdit() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_switchToEntryEdit();
	}
}

void MainWindow::do_deleteEntries() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_deleteEntries();
	}
}

void MainWindow::do_copyTitle() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_copyTitle();
	}
}

void MainWindow::do_copyUsername() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_copyUsername();
	}
}

void MainWindow::do_copyPassword() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_copyPassword();
	}
}

void MainWindow::do_copyURL() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_copyURL();
	}
}

void MainWindow::do_copyNotes() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_copyNotes();
	}
}

void MainWindow::do_copyAttribute(
	const QAction* action
) const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_copyAttribute(
			action
		);
	}
}

void MainWindow::do_performAutoType() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_performAutoType();
	}
}

void MainWindow::do_openUrl() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_openUrl();
	}
}

void MainWindow::do_createGroup() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_createGroup();
	}
}

void MainWindow::do_switchToGroupEdit() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_switchToGroupEdit();
	}
}

void MainWindow::do_deleteGroup() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_deleteGroup();
	}
}

void MainWindow::do_openSearch() const
{
	if(this->currentDatabaseWidget)
	{
		this->currentDatabaseWidget->do_openSearch();
	}
}
