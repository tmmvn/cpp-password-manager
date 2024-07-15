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
#include "DatabaseTabWidget.h"
#include <QLockFile>
#include <QSaveFile>
#include <QTabWidget>
#include "core/Config.h"
#include "core/Database.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "format/CsvExporter.h"
#include "gui/Clipboard.h"
#include "gui/DatabaseWidget.h"
#include "gui/DatabaseWidgetStateSync.h"
#include "gui/DragTabBar.h"
#include "gui/FileDialog.h"
#include "gui/MessageBox.h"
#include "gui/group/GroupView.h"

DatabaseManagerStruct::DatabaseManagerStruct()
	: dbWidget(
		nullptr
	),
	lockFile(
		nullptr
	),
	saveToFilename(
		false
	),
	modified(
		false
	),
	readOnly(
		false
	)
{
}

const int DatabaseTabWidget::LastDatabasesCount = 5;

DatabaseTabWidget::DatabaseTabWidget(
	QWidget* parent
)
	: QTabWidget(
		parent
	),
	dbWidgetSateSync(
		new DatabaseWidgetStateSync(
			this
		)
	)
{
	const auto tabBar_ = new DragTabBar(
		this
	);
	this->setTabBar(
		tabBar_
	);
	this->setDocumentMode(
		true
	);
	this->connect(
		this,
		&DatabaseTabWidget::tabCloseRequested,
		this,
		&DatabaseTabWidget::do_closeDatabase
	);
	this->connect(
		this,
		&DatabaseTabWidget::currentChanged,
		this,
		&DatabaseTabWidget::do_emitActivateDatabaseChanged
	);
	this->connect(
		this,
		&DatabaseTabWidget::sig_activateDatabaseChanged,
		this->dbWidgetSateSync,
		&DatabaseWidgetStateSync::do_setActive
	);
}

DatabaseTabWidget::~DatabaseTabWidget()
{
	QHashIterator i_(
		this->dbList
	);
	while(i_.hasNext())
	{
		i_.next();
		this->deleteDatabase(
			i_.key()
		);
	}
}

void DatabaseTabWidget::do_toggleTabbar() const
{
	if(this->count() > 1)
	{
		if(!this->tabBar()->isVisible())
		{
			this->tabBar()->show();
		}
	}
	else
	{
		if(this->tabBar()->isVisible())
		{
			this->tabBar()->hide();
		}
	}
}

void DatabaseTabWidget::do_newDatabase()
{
	DatabaseManagerStruct dbStruct_;
	const auto db_ = new Database();
	db_->getRootGroup()->setName(
		tr(
			"Root"
		)
	);
	dbStruct_.dbWidget = new DatabaseWidget(
		db_,
		this
	);
	this->insertDatabase(
		db_,
		dbStruct_
	);
	dbStruct_.dbWidget->do_switchToMasterKeyChange();
}

void DatabaseTabWidget::do_openDatabase()
{
	const QString filter_ = QString(
		"%1 (*.kdbx);;%2 (*)"
	).arg(
		tr(
			"KeePass 2 Database"
		),
		tr(
			"All files"
		)
	);
	if(const QString fileName_ = FileDialog::getInstance()->getOpenFileName(
			this,
			tr(
				"Open database"
			),
			QString(),
			filter_
		);
		!fileName_.isEmpty())
	{
		this->openDatabase(
			fileName_
		);
	}
}

void DatabaseTabWidget::openDatabase(
	const QString &fileName,
	const QString &pw,
	const QString &keyFile
)
{
	const QFileInfo fileInfo_(
		fileName
	);
	const QString canonicalFilePath_ = fileInfo_.canonicalFilePath();
	if(canonicalFilePath_.isEmpty())
	{
		MessageBox::warning(
			this,
			tr(
				"Warning"
			),
			tr(
				"File not found!"
			)
		);
		return;
	}
	QHashIterator i_(
		this->dbList
	);
	while(i_.hasNext())
	{
		i_.next();
		if(i_.value().canonicalFilePath == canonicalFilePath_)
		{
			this->setCurrentIndex(
				this->databaseIndex(
					i_.key()
				)
			);
			return;
		}
	}
	DatabaseManagerStruct dbStruct_;
	// test if we can read/write or read the file
	QFile file_(
		fileName
	);
	if(!file_.open(
		QIODevice::ReadWrite
	))
	{
		if(!file_.open(
			QIODevice::ReadOnly
		))
		{
			MessageBox::warning(
				this,
				tr(
					"Error"
				),
				tr(
					"Unable to open the database."
				).append(
					"\n"
				).append(
					file_.errorString()
				)
			);
			return;
		}
		// can only open read-only
		dbStruct_.readOnly = true;
	}
	file_.close();
	auto lockFile_ = new QLockFile(
		QString(
			"%1/.%2.lock"
		).arg(
			fileInfo_.canonicalPath(),
			fileInfo_.fileName()
		)
	);
	lockFile_->setStaleLockTime(
		0
	);
	if(!dbStruct_.readOnly && !lockFile_->tryLock())
	{
		// for now silently ignore if we can't create a lock file
		// due to lack of permissions
		if(lockFile_->error() != QLockFile::PermissionError)
		{
			if(const QMessageBox::StandardButton result_ = MessageBox::question(
					this,
					tr(
						"Open database"
					),
					tr(
						"The database you are trying to open is locked by another instance of KeePassX.\n"
						"Do you want to open it anyway? Alternatively the database is opened read-only."
					),
					QMessageBox::Yes | QMessageBox::No
				);
				result_ == QMessageBox::No)
			{
				dbStruct_.readOnly = true;
				delete lockFile_;
				lockFile_ = nullptr;
			}
			else
			{
				// take over the lock file if possible
				if(lockFile_->removeStaleLockFile())
				{
					lockFile_->tryLock();
				}
			}
		}
	}
	const auto db_ = new Database();
	dbStruct_.dbWidget = new DatabaseWidget(
		db_,
		this
	);
	dbStruct_.lockFile = lockFile_;
	dbStruct_.saveToFilename = !dbStruct_.readOnly;
	dbStruct_.filePath = fileInfo_.absoluteFilePath();
	dbStruct_.canonicalFilePath = canonicalFilePath_;
	dbStruct_.fileName = fileInfo_.fileName();
	insertDatabase(
		db_,
		dbStruct_
	);
	updateLastDatabases(
		dbStruct_.filePath
	);
	if(!pw.isNull() || !keyFile.isEmpty())
	{
		dbStruct_.dbWidget->do_switchToOpenDatabase(
			dbStruct_.filePath,
			pw,
			keyFile
		);
	}
	else
	{
		dbStruct_.dbWidget->do_switchToOpenDatabase(
			dbStruct_.filePath
		);
	}
}

void DatabaseTabWidget::do_importKeePass1Database()
{
	const QString fileName_ = FileDialog::getInstance()->getOpenFileName(
		this,
		tr(
			"Open KeePass 1 database"
		),
		QString(),
		tr(
			"KeePass 1 database"
		) + " (*.kdb);;" + tr(
			"All files (*)"
		)
	);
	if(fileName_.isEmpty())
	{
		return;
	}
	const auto db_ = new Database();
	DatabaseManagerStruct dbStruct_;
	dbStruct_.dbWidget = new DatabaseWidget(
		db_,
		this
	);
	dbStruct_.modified = true;
	insertDatabase(
		db_,
		dbStruct_
	);
}

bool DatabaseTabWidget::closeDatabase(
	Database* db
)
{
	if(db == nullptr)
	{
		return false;
	}
	const DatabaseManagerStruct &dbStruct_ = this->dbList.value(
		db
	);
	const int index_ = this->databaseIndex(
		db
	);
	if(index_ == -1)
	{
		return false;
	};
	QString dbName_ = this->tabText(
		index_
	);
	if(dbName_.right(
		1
	) == "*")
	{
		dbName_.chop(
			1
		);
	}
	if(dbStruct_.dbWidget->isInEditMode() && db->hasKey() && dbStruct_.dbWidget
		->isEditWidgetModified())
	{
		if(const QMessageBox::StandardButton result_ = MessageBox::question(
				this,
				tr(
					"Close?"
				),
				tr(
					"\"%1\" is in edit mode.\nDiscard changes and close anyway?"
				).arg(
					dbName_
				),
				QMessageBox::Discard | QMessageBox::Cancel,
				QMessageBox::Cancel
			);
			result_ == QMessageBox::Cancel)
		{
			return false;
		}
	}
	if(dbStruct_.modified)
	{
		if(Config::getInstance()->get(
			"AutoSaveOnExit"
		).toBool())
		{
			if(!this->saveDatabase(
				db
			))
			{
				return false;
			}
		}
		else
		{
			if(const QMessageBox::StandardButton result_ = MessageBox::question(
					this,
					tr(
						"Save changes?"
					),
					tr(
						"\"%1\" was modified.\nSave changes?"
					).arg(
						dbName_
					),
					QMessageBox::Yes | QMessageBox::Discard |
					QMessageBox::Cancel,
					QMessageBox::Yes
				);
				result_ == QMessageBox::Yes)
			{
				if(!this->saveDatabase(
					db
				))
				{
					return false;
				}
			}
			else if(result_ == QMessageBox::Cancel)
			{
				return false;
			}
		}
	}
	this->deleteDatabase(
		db
	);
	return true;
}

void DatabaseTabWidget::deleteDatabase(
	Database* db
)
{
	const DatabaseManagerStruct dbStruct_ = this->dbList.value(
		db
	);
	const bool emitDatabaseWithFileClosed_ = dbStruct_.saveToFilename;
	const QString filePath_ = dbStruct_.filePath;
	const int index_ = this->databaseIndex(
		db
	);
	this->removeTab(
		index_
	);
	this->do_toggleTabbar();
	this->dbList.remove(
		db
	);
	delete dbStruct_.lockFile;
	delete dbStruct_.dbWidget;
	delete db;
	if(emitDatabaseWithFileClosed_)
	{
		this->sig_databaseWithFileClosed(
			filePath_
		);
	}
}

bool DatabaseTabWidget::do_closeAllDatabases()
{
	while(!this->dbList.isEmpty())
	{
		if(!this->do_closeDatabase())
		{
			return false;
		}
	}
	return true;
}

bool DatabaseTabWidget::saveDatabase(
	Database* db
)
{
	if(DatabaseManagerStruct &dbStruct_ = this->dbList[db];
		dbStruct_.saveToFilename)
	{
		QSaveFile saveFile_(
			dbStruct_.canonicalFilePath
		);
		if(saveFile_.open(
			QIODevice::WriteOnly
		))
		{
			this->writer.writeDatabase(
				&saveFile_,
				db
			);
			if(this->writer.hasError())
			{
				MessageBox::critical(
					this,
					this->tr(
						"Error"
					),
					this->tr(
						"Writing the database failed."
					) + "\n\n" + this->writer.getErrorString()
				);
				return false;
			}
			if(!saveFile_.commit())
			{
				MessageBox::critical(
					this,
					this->tr(
						"Error"
					),
					this->tr(
						"Writing the database failed."
					) + "\n\n" + saveFile_.errorString()
				);
				return false;
			}
		}
		else
		{
			MessageBox::critical(
				this,
				this->tr(
					"Error"
				),
				this->tr(
					"Writing the database failed."
				) + "\n\n" + saveFile_.errorString()
			);
			return false;
		}
		dbStruct_.modified = false;
		this->do_updateTabName(
			db
		);
		return true;
	}
	return this->saveDatabaseAs(
		db
	);
}

bool DatabaseTabWidget::saveDatabaseAs(
	Database* db
)
{
	DatabaseManagerStruct &dbStruct_ = this->dbList[db];
	QString oldFileName_;
	if(dbStruct_.saveToFilename)
	{
		oldFileName_ = dbStruct_.filePath;
	}
	else
	{
		oldFileName_ = this->tr(
			"New database"
		).append(
			".kdbx"
		);
	}
	if(const QString fileName_ = FileDialog::getInstance()->getSaveFileName(
			this,
			this->tr(
				"Save database as"
			),
			oldFileName_,
			this->tr(
				"KeePass 2 Database"
			).append(
				" (*.kdbx)"
			),
			nullptr,
			"kdbx"
		);
		!fileName_.isEmpty())
	{
		QFileInfo fileInfo_(
			fileName_
		);
		QString lockFilePath_;
		if(fileInfo_.exists())
		{
			// returns empty string when file doesn't exist
			lockFilePath_ = fileInfo_.canonicalPath();
		}
		else
		{
			lockFilePath_ = fileInfo_.absolutePath();
		}
		const QString lockFileName_ = QString(
			"%1/.%2.lock"
		).arg(
			lockFilePath_,
			fileInfo_.fileName()
		);
		std::unique_ptr<QLockFile> lockFile_(
			new QLockFile(
				lockFileName_
			)
		);
		lockFile_->setStaleLockTime(
			0
		);
		if(!lockFile_->tryLock())
		{
			// for now silently ignore if we can't create a lock file
			// due to lack of permissions
			if(lockFile_->error() != QLockFile::PermissionError)
			{
				if(const QMessageBox::StandardButton result_ =
						MessageBox::question(
							this,
							this->tr(
								"Save database as"
							),
							this->tr(
								"The database you are trying to save as is locked by another instance of KeePassX.\n"
								"Do you want to save it anyway?"
							),
							QMessageBox::Yes | QMessageBox::No
						);
					result_ == QMessageBox::No)
				{
					return false;
				}
				// take over the lock file if possible
				if(lockFile_->removeStaleLockFile())
				{
					lockFile_->tryLock();
				}
			}
		}
		QSaveFile saveFile_(
			fileName_
		);
		if(!saveFile_.open(
			QIODevice::WriteOnly
		))
		{
			MessageBox::critical(
				this,
				this->tr(
					"Error"
				),
				this->tr(
					"Writing the database failed."
				) + "\n\n" + saveFile_.errorString()
			);
			return false;
		}
		this->writer.writeDatabase(
			&saveFile_,
			db
		);
		if(this->writer.hasError())
		{
			MessageBox::critical(
				this,
				this->tr(
					"Error"
				),
				this->tr(
					"Writing the database failed."
				) + "\n\n" + writer.getErrorString()
			);
			return false;
		}
		if(!saveFile_.commit())
		{
			MessageBox::critical(
				this,
				this->tr(
					"Error"
				),
				this->tr(
					"Writing the database failed."
				) + "\n\n" + saveFile_.errorString()
			);
			return false;
		}
		// refresh fileinfo since the file didn't exist before
		fileInfo_.refresh();
		dbStruct_.modified = false;
		dbStruct_.saveToFilename = true;
		dbStruct_.readOnly = false;
		dbStruct_.filePath = fileInfo_.absoluteFilePath();
		dbStruct_.canonicalFilePath = fileInfo_.canonicalFilePath();
		dbStruct_.fileName = fileInfo_.fileName();
		dbStruct_.dbWidget->updateFilename(
			dbStruct_.filePath
		);
		delete dbStruct_.lockFile;
		dbStruct_.lockFile = lockFile_.release();
		this->do_updateTabName(
			db
		);
		this->updateLastDatabases(
			dbStruct_.filePath
		);
		return true;
	}
	return false;
}

bool DatabaseTabWidget::do_closeDatabase(
	int index
)
{
	if(index == -1)
	{
		index = currentIndex();
	}
	this->setCurrentIndex(
		index
	);
	return this->closeDatabase(
		this->indexDatabase(
			index
		)
	);
}

void DatabaseTabWidget::do_closeDatabaseFromSender()
{
	if(this->sender() == nullptr)
	{
		return;
	}
	const auto dbWidget_ = static_cast<DatabaseWidget*>(this->sender());
	Database* db_ = this->databaseFromDatabaseWidget(
		dbWidget_
	);
	const int index_ = this->databaseIndex(
		db_
	);
	this->setCurrentIndex(
		index_
	);
	this->closeDatabase(
		db_
	);
}

bool DatabaseTabWidget::do_saveDatabase(
	int index
)
{
	if(index == -1)
	{
		index = this->currentIndex();
	}
	return this->saveDatabase(
		this->indexDatabase(
			index
		)
	);
}

bool DatabaseTabWidget::do_saveDatabaseAs(
	int index
)
{
	if(index == -1)
	{
		index = this->currentIndex();
	}
	return this->saveDatabaseAs(
		this->indexDatabase(
			index
		)
	);
}

void DatabaseTabWidget::do_exportToCsv()
{
	const Database* db_ = this->indexDatabase(
		this->currentIndex()
	);
	if(db_ == nullptr)
	{
		return;
	}
	const QString fileName_ = FileDialog::getInstance()->getSaveFileName(
		this,
		this->tr(
			"Export database to CSV file"
		),
		QString(),
		this->tr(
			"CSV file"
		).append(
			" (*.csv)"
		),
		nullptr,
		"csv"
	);
	if(fileName_.isEmpty())
	{
		return;
	}
	if(CsvExporter csvExporter_;
		!csvExporter_.exportDatabase(
			fileName_,
			db_
		))
	{
		MessageBox::critical(
			this,
			this->tr(
				"Error"
			),
			this->tr(
				"Writing the CSV file failed."
			) + "\n\n" + csvExporter_.getErrorString()
		);
	}
}

void DatabaseTabWidget::do_changeMasterKey()
{
	this->getCurrentDatabaseWidget()->do_switchToMasterKeyChange();
}

void DatabaseTabWidget::do_changeDatabaseSettings()
{
	this->getCurrentDatabaseWidget()->do_switchToDatabaseSettings();
}

bool DatabaseTabWidget::do_readOnly(
	int index
) const
{
	if(index == -1)
	{
		index = this->currentIndex();
	}
	return this->indexDatabaseManagerStruct(
		index
	).readOnly;
}

void DatabaseTabWidget::do_updateTabName(
	Database* db
)
{
	const int index_ = this->databaseIndex(
		db
	);
	if(index_ == -1)
	{
		return;
	}
	const DatabaseManagerStruct &dbStruct_ = this->dbList.value(
		db
	);
	QString tabName_;
	if(dbStruct_.saveToFilename || dbStruct_.readOnly)
	{
		if(db->getMetadata()->getName().isEmpty())
		{
			tabName_ = dbStruct_.fileName;
		}
		else
		{
			tabName_ = db->getMetadata()->getName();
		}
		this->setTabToolTip(
			index_,
			dbStruct_.filePath
		);
	}
	else
	{
		if(db->getMetadata()->getName().isEmpty())
		{
			tabName_ = this->tr(
				"New database"
			);
		}
		else
		{
			tabName_ = QString(
				"%1 [%2]"
			).arg(
				db->getMetadata()->getName(),
				this->tr(
					"New database"
				)
			);
		}
	}
	if(dbStruct_.dbWidget->getCurrentMode() == DatabaseWidget::LockedMode)
	{
		tabName_.append(
			QString(
				" [%1]"
			).arg(
				this->tr(
					"locked"
				)
			)
		);
	}
	if(dbStruct_.modified)
	{
		tabName_.append(
			"*"
		);
	}
	this->setTabText(
		index_,
		tabName_
	);
	this->sig_tabNameChanged();
}

void DatabaseTabWidget::do_updateTabNameFromDbSender()
{
	if(!qobject_cast<Database*>(
		this->sender()
	))
	{
		qWarning() << "Invalid sender object in the current context.";
		return; // Or return an error code
	}
	this->do_updateTabName(
		static_cast<Database*>(this->sender())
	);
}

void DatabaseTabWidget::do_updateTabNameFromDbWidgetSender()
{
	if(!qobject_cast<DatabaseWidget*>(
		this->sender()
	))
	{
		qWarning() <<
			"Invalid sender object in the current context. Expected DatabaseWidget.";
		return; // Or return an error code
	}
	if(!this->databaseFromDatabaseWidget(
		qobject_cast<DatabaseWidget*>(
			this->sender()
		)
	))
	{
		qWarning() << "DatabaseWidget not found in the database list.";
		return; // Or return an error code
	}
	const auto dbWidget_ = static_cast<DatabaseWidget*>(this->sender());
	this->do_updateTabName(
		this->databaseFromDatabaseWidget(
			dbWidget_
		)
	);
}

int DatabaseTabWidget::databaseIndex(
	Database* db
) const
{
	const QWidget* dbWidget_ = this->dbList.value(
		db
	).dbWidget;
	return indexOf(
		dbWidget_
	);
}

Database* DatabaseTabWidget::indexDatabase(
	const int index
) const
{
	const QWidget* dbWidget_ = this->widget(
		index
	);
	QHashIterator i_(
		this->dbList
	);
	while(i_.hasNext())
	{
		i_.next();
		if(i_.value().dbWidget == dbWidget_)
		{
			return i_.key();
		}
	}
	return nullptr;
}

DatabaseManagerStruct DatabaseTabWidget::indexDatabaseManagerStruct(
	const int index
) const
{
	const QWidget* dbWidget_ = this->widget(
		index
	);
	QHashIterator i_(
		this->dbList
	);
	while(i_.hasNext())
	{
		i_.next();
		if(i_.value().dbWidget == dbWidget_)
		{
			return i_.value();
		}
	}
	return DatabaseManagerStruct();
}

Database* DatabaseTabWidget::databaseFromDatabaseWidget(
	const DatabaseWidget* dbWidget
) const
{
	QHashIterator i_(
		this->dbList
	);
	while(i_.hasNext())
	{
		i_.next();
		if(i_.value().dbWidget == dbWidget)
		{
			return i_.key();
		}
	}
	return nullptr;
}

void DatabaseTabWidget::insertDatabase(
	Database* db,
	const DatabaseManagerStruct &dbStruct
)
{
	this->dbList.insert(
		db,
		dbStruct
	);
	this->addTab(
		dbStruct.dbWidget,
		""
	);
	this->do_toggleTabbar();
	this->do_updateTabName(
		db
	);
	const int index_ = this->databaseIndex(
		db
	);
	this->setCurrentIndex(
		index_
	);
	this->connectDatabase(
		db
	);
	this->connect(
		dbStruct.dbWidget,
		&DatabaseWidget::sig_closeRequest,
		this,
		&DatabaseTabWidget::do_closeDatabaseFromSender
	);
	this->connect(
		dbStruct.dbWidget,
		&DatabaseWidget::sig_databaseChanged,
		this,
		&DatabaseTabWidget::do_changeDatabase
	);
	this->connect(
		dbStruct.dbWidget,
		&DatabaseWidget::sig_unlockedDatabase,
		this,
		&DatabaseTabWidget::do_updateTabNameFromDbWidgetSender
	);
}

DatabaseWidget* DatabaseTabWidget::getCurrentDatabaseWidget()
{
	if(Database* db_ = this->indexDatabase(
		this->currentIndex()
	))
	{
		return this->dbList[db_].dbWidget;
	}
	return nullptr;
}

bool DatabaseTabWidget::hasLockableDatabases() const
{
	QHashIterator i_(
		this->dbList
	);
	while(i_.hasNext())
	{
		i_.next();
		if(const DatabaseWidget::Mode mode_ = i_.value().dbWidget->
				getCurrentMode();
			(mode_ == DatabaseWidget::ViewMode || mode_ ==
				DatabaseWidget::EditMode) && i_.value().dbWidget->dbHasKey())
		{
			return true;
		}
	}
	return false;
}

void DatabaseTabWidget::do_lockDatabases()
{
	Clipboard::getInstance()->do_clearCopiedText();
	for(auto i_ = 0; i_ < count(); i_++)
	{
		const auto dbWidget_ = static_cast<DatabaseWidget*>(this->widget(
			i_
		));
		Database* db_ = this->databaseFromDatabaseWidget(
			dbWidget_
		);
		const DatabaseWidget::Mode mode_ = dbWidget_->getCurrentMode();
		if((mode_ != DatabaseWidget::ViewMode && mode_ !=
			DatabaseWidget::EditMode) || !dbWidget_->dbHasKey())
		{
			continue;
		}
		// show the correct tab widget before we are asking questions about it
		this->setCurrentWidget(
			dbWidget_
		);
		if(mode_ == DatabaseWidget::EditMode && dbWidget_->
			isEditWidgetModified())
		{
			if(const QMessageBox::StandardButton result_ = MessageBox::question(
					this,
					this->tr(
						"Lock database"
					),
					this->tr(
						"Can't lock the database as you are currently editing it.\nPlease press cancel to finish your changes or discard them."
					),
					QMessageBox::Discard | QMessageBox::Cancel,
					QMessageBox::Cancel
				);
				result_ == QMessageBox::Cancel)
			{
				continue;
			}
		}
		if(this->dbList[db_].modified && !this->dbList[db_].saveToFilename)
		{
			if(const QMessageBox::StandardButton result_ = MessageBox::question(
					this,
					this->tr(
						"Lock database"
					),
					this->tr(
						"This database has never been saved.\nYou can save the database or stop locking it."
					),
					QMessageBox::Save | QMessageBox::Cancel,
					QMessageBox::Cancel
				);
				result_ == QMessageBox::Save)
			{
				if(!this->saveDatabase(
					db_
				))
				{
					continue;
				}
			}
			else if(result_ == QMessageBox::Cancel)
			{
				continue;
			}
		}
		else if(this->dbList[db_].modified)
		{
			if(const QMessageBox::StandardButton result_ = MessageBox::question(
					this,
					this->tr(
						"Lock database"
					),
					this->tr(
						"This database has been modified.\nDo you want to save the database before locking it?\nOtherwise your changes are lost."
					),
					QMessageBox::Save | QMessageBox::Discard |
					QMessageBox::Cancel,
					QMessageBox::Cancel
				);
				result_ == QMessageBox::Save)
			{
				if(!this->saveDatabase(
					db_
				))
				{
					continue;
				}
			}
			else if(result_ == QMessageBox::Discard)
			{
				this->dbList[db_].modified = false;
			}
			else if(result_ == QMessageBox::Cancel)
			{
				continue;
			}
		}
		dbWidget_->lock();
		// database has changed so we can't use the db variable anymore
		this->do_updateTabName(
			dbWidget_->getDatabase()
		);
	}
}

void DatabaseTabWidget::do_modified()
{
	if(!qobject_cast<Database*>(
		this->sender()
	))
	{
		qWarning() <<
			"Invalid sender object in the current context. Expected Database.";
		return; // Or return an error code
	}
	const auto db_ = static_cast<Database*>(this->sender());
	DatabaseManagerStruct &dbStruct_ = this->dbList[db_];
	if(Config::getInstance()->get(
		"AutoSaveAfterEveryChange"
	).toBool() && dbStruct_.saveToFilename)
	{
		this->saveDatabase(
			db_
		);
		return;
	}
	if(!dbStruct_.modified)
	{
		dbStruct_.modified = true;
		this->do_updateTabName(
			db_
		);
	}
}

void DatabaseTabWidget::updateLastDatabases(
	const QString &filename
) const
{
	if(!Config::getInstance()->get(
		"RememberLastDatabases"
	).toBool())
	{
		Config::getInstance()->set(
			"LastDatabases",
			QVariant()
		);
	}
	else
	{
		QStringList lastDatabases_ = Config::getInstance()->get(
			"LastDatabases",
			QVariant()
		).toStringList();
		lastDatabases_.prepend(
			filename
		);
		lastDatabases_.removeDuplicates();
		while(lastDatabases_.count() > this->LastDatabasesCount)
		{
			lastDatabases_.removeLast();
		}
		Config::getInstance()->set(
			"LastDatabases",
			lastDatabases_
		);
	}
}

void DatabaseTabWidget::do_changeDatabase(
	Database* newDb
)
{
	if(this->sender() == nullptr)
	{
		return;
	}
	if(this->dbList.contains(
		newDb
	))
	{
		return;
	}
	const auto dbWidget_ = static_cast<DatabaseWidget*>(this->sender());
	Database* oldDb_ = this->databaseFromDatabaseWidget(
		dbWidget_
	);
	const DatabaseManagerStruct dbStruct_ = this->dbList[oldDb_];
	this->dbList.remove(
		oldDb_
	);
	this->dbList.insert(
		newDb,
		dbStruct_
	);
	this->do_updateTabName(
		newDb
	);
	this->connectDatabase(
		newDb,
		oldDb_
	);
}

void DatabaseTabWidget::do_emitActivateDatabaseChanged()
{
	this->sig_activateDatabaseChanged(
		this->getCurrentDatabaseWidget()
	);
}

void DatabaseTabWidget::connectDatabase(
	Database* newDb,
	const Database* oldDb
) const
{
	if(oldDb)
	{
		if(!oldDb->disconnect(
			this
		))
		{
			return;
		}
	}
	this->connect(
		newDb,
		&Database::sig_nameTextChanged,
		this,
		&DatabaseTabWidget::do_updateTabNameFromDbSender
	);
	this->connect(
		newDb,
		&Database::sig_modified,
		this,
		&DatabaseTabWidget::do_modified
	);
	newDb->setEmitModified(
		true
	);
}
