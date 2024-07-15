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
#include "DatabaseRepairWidget.h"
#include <QFileInfo>
#include "ui_DatabaseOpenWidget.h"
#include "core/Database.h"
#include "core/Metadata.h"
#include "format/KeePass2Repair.h"
#include "gui/MessageBox.h"
#include "keys/FileKey.h"
#include "keys/PasswordKey.h"

DatabaseRepairWidget::DatabaseRepairWidget(
	QWidget* parent
)
	: DatabaseOpenWidget(
		parent
	)
{
	this->ui->labelHeadline->setText(
		tr(
			"Repair database"
		)
	);
	this->connect(
		this,
		&DatabaseRepairWidget::sig_editFinished,
		this,
		&DatabaseRepairWidget::do_processEditFinished
	);
}

void DatabaseRepairWidget::do_openDatabase()
{
	CompositeKey masterKey_;
	if(this->ui->checkPassword->isChecked())
	{
		masterKey_.addKey(
			PasswordKey(
				this->ui->editPassword->text()
			)
		);
	}
	if(this->ui->checkKeyFile->isChecked())
	{
		FileKey key_;
		const QString keyFilename_ = this->ui->comboKeyFile->currentText();
		QString errorMsg_;
		if(!key_.load(
			keyFilename_,
			&errorMsg_
		))
		{
			MessageBox::warning(
				this,
				tr(
					"Error"
				),
				tr(
					"Can't open key file"
				).append(
					":\n"
				).append(
					errorMsg_
				)
			);
			this->sig_editFinished(
				false
			);
			return;
		}
		masterKey_.addKey(
			key_
		);
	}
	KeePass2Repair repair_;
	QFile file_(
		filename
	);
	if(!file_.open(
		QIODevice::ReadOnly
	))
	{
		MessageBox::warning(
			this,
			this->tr(
				"Error"
			),
			this->tr(
				"Unable to open the database."
			).append(
				"\n"
			).append(
				file_.errorString()
			)
		);
		this->sig_editFinished(
			false
		);
		return;
	}
	if(this->db)
	{
		delete this->db;
	}
	QApplication::setOverrideCursor(
		QCursor(
			Qt::WaitCursor
		)
	);
	const KeePass2Repair::RepairResult repairResult_ = repair_.repairDatabase(
		&file_,
		masterKey_
	);
	QApplication::restoreOverrideCursor();
	switch(repairResult_)
	{
		case KeePass2Repair::NothingTodo:
			MessageBox::information(
				this,
				tr(
					"Error"
				),
				tr(
					"Database opened fine. Nothing to do."
				)
			);
			this->sig_editFinished(
				false
			);
			return;
		case KeePass2Repair::UnableToOpen:
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
					repair_.getErrorString()
				)
			);
			this->sig_editFinished(
				false
			);
			return;
		case KeePass2Repair::RepairSuccess:
			this->db = repair_.getDatabase();
			MessageBox::warning(
				this,
				tr(
					"Success"
				),
				tr(
					"The database has been successfully repaired\nYou can now save it."
				)
			);
			this->sig_editFinished(
				true
			);
			return;
		case KeePass2Repair::RepairFailed:
			MessageBox::warning(
				this,
				tr(
					"Error"
				),
				tr(
					"Unable to repair the database."
				)
			);
			this->sig_editFinished(
				false
			);
	}
}

void DatabaseRepairWidget::do_processEditFinished(
	const bool result
)
{
	if(result)
	{
		this->sig_success();
	}
	else
	{
		this->sig_error();
	}
}
