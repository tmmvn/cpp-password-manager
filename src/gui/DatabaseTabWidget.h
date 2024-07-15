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
#ifndef KEEPASSX_DATABASETABWIDGET_H
#define KEEPASSX_DATABASETABWIDGET_H
#include <QHash>
#include <QTabWidget>
#include "format/KeePass2Writer.h"
#include "gui/DatabaseWidget.h"
class DatabaseWidget;
class DatabaseWidgetStateSync;
class DatabaseOpenWidget;
class QFile;
class QLockFile;

struct DatabaseManagerStruct
{
	DatabaseManagerStruct();
	DatabaseWidget* dbWidget;
	QLockFile* lockFile;
	QString filePath;
	QString canonicalFilePath;
	QString fileName;
	bool saveToFilename;
	bool modified;
	bool readOnly;
};

Q_DECLARE_TYPEINFO(
	DatabaseManagerStruct,
	Q_MOVABLE_TYPE
);

class DatabaseTabWidget:public QTabWidget
{
	Q_OBJECT public:
	explicit DatabaseTabWidget(
		QWidget* parent = nullptr
	);
	virtual ~DatabaseTabWidget() override;
	void openDatabase(
		const QString &fileName,
		const QString &pw = QString(),
		const QString &keyFile = QString()
	);
	DatabaseWidget* getCurrentDatabaseWidget();
	bool hasLockableDatabases() const;
	static const int LastDatabasesCount;
public Q_SLOTS:
	void do_newDatabase();
	void do_openDatabase();
	void do_importKeePass1Database();
	bool do_saveDatabase(
		int index = -1
	);
	bool do_saveDatabaseAs(
		int index = -1
	);
	void do_exportToCsv();
	bool do_closeDatabase(
		int index = -1
	);
	void do_closeDatabaseFromSender();
	bool do_closeAllDatabases();
	void do_changeMasterKey();
	void do_changeDatabaseSettings();
	bool do_readOnly(
		int index = -1
	) const;
	void do_lockDatabases();
Q_SIGNALS:
	void sig_tabNameChanged();
	void sig_databaseWithFileClosed(
		QString filePath
	);
	void sig_activateDatabaseChanged(
		DatabaseWidget* dbWidget
	);
private Q_SLOTS:
	void do_updateTabName(
		Database* db
	);
	void do_updateTabNameFromDbSender();
	void do_updateTabNameFromDbWidgetSender();
	void do_modified();
	void do_toggleTabbar() const;
	void do_changeDatabase(
		Database* newDb
	);
	void do_emitActivateDatabaseChanged();
private:
	bool saveDatabase(
		Database* db
	);
	bool saveDatabaseAs(
		Database* db
	);
	bool closeDatabase(
		Database* db
	);
	void deleteDatabase(
		Database* db
	);
	int databaseIndex(
		Database* db
	) const;
	Database* indexDatabase(
		int index
	) const;
	DatabaseManagerStruct indexDatabaseManagerStruct(
		int index
	) const;
	Database* databaseFromDatabaseWidget(
		const DatabaseWidget* dbWidget
	) const;
	void insertDatabase(
		Database* db,
		const DatabaseManagerStruct &dbStruct
	);
	void updateLastDatabases(
		const QString &filename
	) const;
	void connectDatabase(
		Database* newDb,
		const Database* oldDb = nullptr
	) const;
	KeePass2Writer writer;
	QHash<Database*, DatabaseManagerStruct> dbList;
	DatabaseWidgetStateSync* dbWidgetSateSync;
};
#endif // KEEPASSX_DATABASETABWIDGET_H
