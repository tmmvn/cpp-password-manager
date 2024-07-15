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
#ifndef KEEPASSX_DATABASEWIDGET_H
#define KEEPASSX_DATABASEWIDGET_H
#include <QScopedPointer>
#include <QStackedWidget>
#include <core/Database.h>
#include "core/UUID.h"
#include "gui/entry/EntryModel.h"
class ChangeMasterKeyWidget;
class DatabaseOpenWidget;
class DatabaseSettingsWidget;
class EditEntryWidget;
class EditGroupWidget;
class Entry;
class EntryView;
class Group;
class GroupView;
class QFile;
class QMenu;
class QSplitter;
class UnlockDatabaseWidget;

namespace Ui
{
	class SearchWidget;
}

class DatabaseWidget final:public QStackedWidget
{
	Q_OBJECT public:
	enum Mode: uint8_t
	{
		None,
		ViewMode,
		EditMode,
		LockedMode
	};

	explicit DatabaseWidget(
		Database* db,
		QWidget* parent = nullptr
	);
	virtual ~DatabaseWidget() override;
	Database* getDatabase() const;
	bool dbHasKey() const;
	bool canDeleteCurrentGroup() const;
	bool isInSearchMode() const;
	int addDbWidget(
		QWidget* w
	);
	static void setCurrentDbIndex(
		int index
	);
	void setCurrentDbWidget(
		QWidget* widget
	);
	Mode getCurrentMode() const;
	void lock();
	void updateFilename(
		const QString &filename
	);
	int getNumberOfSelectedEntries() const;
	QStringList getCustomEntryAttributes() const;
	bool isGroupSelected() const;
	bool isInEditMode() const;
	bool isEditWidgetModified() const;
	QList<int> getSplitterSizes() const;
	void setSplitterSizes(
		const QList<int> &sizes
	) const;
	QList<int> getEntryHeaderViewSizes() const;
	void setEntryViewHeaderSizes(
		const QList<int> &sizes
	) const;
	void clearAllWidgets() const;
	bool currentEntryHasTitle() const;
	bool currentEntryHasUsername() const;
	bool currentEntryHasPassword() const;
	bool currentEntryHasUrl() const;
	bool currentEntryHasNotes() const;
Q_SIGNALS:
	void sig_closeRequest();
	void sig_currentModeChanged(
		DatabaseWidget::Mode mode
	);
	void sig_groupChanged();
	void sig_entrySelectionChanged();
	void sig_databaseChanged(
		Database* newDb
	);
	void sig_groupContextMenuRequested(
		const QPoint &globalPos
	);
	void sig_entryContextMenuRequested(
		const QPoint &globalPos
	);
	void sig_unlockedDatabase();
	void sig_listModeAboutToActivate();
	void sig_listModeActivated();
	void sig_searchModeAboutToActivate();
	void sig_searchModeActivated();
	void sig_splitterSizesChanged();
	void sig_entryColumnSizesChanged();
protected:
	virtual bool eventFilter(
		QObject* object,
		QEvent* event
	) override;
public Q_SLOTS:
	void do_createEntry();
	void do_cloneEntry() const;
	void do_deleteEntries();
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
	static void do_openUrlForEntry(
		const Entry* entry
	);
	void do_createGroup();
	void do_deleteGroup();
	void do_switchToEntryEdit();
	void do_switchToGroupEdit();
	void do_switchToMasterKeyChange();
	void do_switchToDatabaseSettings();
	void do_switchToOpenDatabase(
		const QString &fileName
	);
	void do_switchToOpenDatabase(
		const QString &fileName,
		const QString &password,
		const QString &keyFile
	);
	void do_openSearch();
private Q_SLOTS:
	void do_entryActivationSignalReceived(
		Entry* entry,
		EntryModel::ModelColumn column
	);
	void do_switchBackToEntryEdit();
	void do_switchToView(
		bool accepted
	);
	void do_switchToHistoryView(
		Entry* entry
	);
	void do_switchToEntryEdit(
		Entry* entry
	);
	void do_switchToEntryEdit(
		Entry* entry,
		bool create
	);
	void do_switchToGroupEdit(
		Group* entry,
		bool create
	);
	void do_emitGroupContextMenuRequested(
		const QPoint &pos
	);
	void do_emitEntryContextMenuRequested(
		const QPoint &pos
	);
	void do_updateMasterKey(
		bool accepted
	);
	void do_openDatabase(
		bool accepted
	);
	void do_unlockDatabase(
		bool accepted
	);
	void do_emitCurrentModeChanged();
	void do_clearLastGroup(
		const Group* group
	);
	void do_search() const;
	void do_startSearch() const;
	void do_startSearchTimer() const;
	void do_showSearch();
	void do_closeSearch();
private:
	void setClipboardTextAndMinimize(
		const QString &text
	) const;
	void setIconFromParent() const;
	void replaceDatabase(
		Database* db
	);
	Database* db;
	const QScopedPointer<Ui::SearchWidget> searchUi;
	QWidget* const searchWidget;
	QWidget* mainWidget;
	EditEntryWidget* editEntryWidget;
	EditEntryWidget* historyEditEntryWidget;
	EditGroupWidget* editGroupWidget;
	ChangeMasterKeyWidget* changeMasterKeyWidget;
	DatabaseSettingsWidget* databaseSettingsWidget;
	DatabaseOpenWidget* databaseOpenWidget;
	UnlockDatabaseWidget* unlockDatabaseWidget;
	QSplitter* splitter;
	GroupView* groupView;
	EntryView* entryView;
	Group* newGroup;
	Entry* newEntry;
	Group* newParent;
	Group* lastGroup;
	QTimer* searchTimer;
	QString filename;
	UUID groupBeforeLock;
};
#endif // KEEPASSX_DATABASEWIDGET_H
