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
#include "DatabaseOpenWidget.h"
#include "ui_DatabaseOpenWidget.h"
#include "core/Config.h"
#include "core/Database.h"
#include "core/FilePath.h"
#include "format/KeePass2Reader.h"
#include "gui/FileDialog.h"
#include "gui/MessageBox.h"
#include "keys/FileKey.h"
#include "keys/PasswordKey.h"

DatabaseOpenWidget::DatabaseOpenWidget(
	QWidget* parent
)
	: DialogWidget(
		parent
	),
	ui(
		new Ui::DatabaseOpenWidget()
	),
	db(
		nullptr
	)
{
	this->ui->setupUi(
		this
	);
	QFont font_ = this->ui->labelHeadline->font();
	font_.setBold(
		true
	);
	font_.setPointSize(
		font_.pointSize() + 2
	);
	this->ui->labelHeadline->setFont(
		font_
	);
	this->ui->buttonBox->button(
		QDialogButtonBox::Ok
	)->setEnabled(
		false
	);
	this->ui->buttonTogglePassword->setIcon(
		FilePath::getInstance()->getOnOffIcon(
			"actions",
			"password-show"
		)
	);
	this->connect(
		this->ui->buttonTogglePassword,
		&QToolButton::toggled,
		this->ui->editPassword,
		&PasswordEdit::do_setShowPassword
	);
	this->connect(
		this->ui->buttonBrowseFile,
		&QPushButton::clicked,
		this,
		&DatabaseOpenWidget::do_browseKeyFile
	);
	this->connect(
		this->ui->editPassword,
		&PasswordEdit::textChanged,
		this,
		&DatabaseOpenWidget::do_activatePassword
	);
	this->connect(
		this->ui->comboKeyFile,
		&QComboBox::editTextChanged,
		this,
		&DatabaseOpenWidget::do_activateKeyFile
	);
	this->connect(
		this->ui->buttonBox,
		&QDialogButtonBox::accepted,
		this,
		&DatabaseOpenWidget::do_openDatabase
	);
	this->connect(
		this->ui->buttonBox,
		&QDialogButtonBox::rejected,
		this,
		&DatabaseOpenWidget::do_reject
	);
}

DatabaseOpenWidget::~DatabaseOpenWidget()
{
}

void DatabaseOpenWidget::load(
	const QString &filename
)
{
	this->filename = filename;
	this->ui->labelFilename->setText(
		this->filename
	);
	if(Config::getInstance()->get(
		"RememberLastKeyFiles"
	).toBool())
	{
		if(QHash<QString, QVariant> lastKeyFiles_ = Config::getInstance()->get(
				"LastKeyFiles"
			).toHash();
			lastKeyFiles_.contains(
				filename
			))
		{
			this->ui->checkKeyFile->setChecked(
				true
			);
			this->ui->comboKeyFile->addItem(
				lastKeyFiles_[filename].toString()
			);
		}
	}
	this->ui->buttonBox->button(
		QDialogButtonBox::Ok
	)->setEnabled(
		true
	);
	this->ui->editPassword->setFocus();
}

Database* DatabaseOpenWidget::database() const
{
	return this->db;
}

void DatabaseOpenWidget::enterKey(
	const QString &pw,
	const QString &keyFile
)
{
	if(!pw.isNull())
	{
		this->ui->editPassword->setText(
			pw
		);
	}
	if(!keyFile.isEmpty())
	{
		this->ui->comboKeyFile->setEditText(
			keyFile
		);
	}
	this->do_openDatabase();
}

void DatabaseOpenWidget::do_openDatabase()
{
	KeePass2Reader reader_;
	const CompositeKey masterKey_ = this->databaseKey();
	QFile file_(
		this->filename
	);
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
	if(this->db)
	{
		delete this->db;
	}
	QApplication::setOverrideCursor(
		QCursor(
			Qt::WaitCursor
		)
	);
	this->db = reader_.readDatabase(
		&file_,
		masterKey_
	);
	QApplication::restoreOverrideCursor();
	if(this->db)
	{
		this->sig_editFinished(
			true
		);
	}
	else
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
				reader_.getErrorString()
			)
		);
		this->ui->editPassword->clear();
	}
}

CompositeKey DatabaseOpenWidget::databaseKey()
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
	QHash<QString, QVariant> lastKeyFiles_ = Config::getInstance()->get(
		"LastKeyFiles"
	).toHash();
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
			return CompositeKey();
		}
		masterKey_.addKey(
			key_
		);
		lastKeyFiles_[this->filename] = keyFilename_;
	}
	else
	{
		lastKeyFiles_.remove(
			this->filename
		);
	}
	if(Config::getInstance()->get(
		"RememberLastKeyFiles"
	).toBool())
	{
		Config::getInstance()->set(
			"LastKeyFiles",
			lastKeyFiles_
		);
	}
	return masterKey_;
}

void DatabaseOpenWidget::do_reject()
{
	this->sig_editFinished(
		false
	);
}

void DatabaseOpenWidget::do_activatePassword() const
{
	this->ui->checkPassword->setChecked(
		true
	);
}

void DatabaseOpenWidget::do_activateKeyFile() const
{
	this->ui->checkKeyFile->setChecked(
		true
	);
}

void DatabaseOpenWidget::do_browseKeyFile()
{
	const QString filters_ = QString(
		"%1 (*);;%2 (*.key)"
	).arg(
		tr(
			"All files"
		),
		tr(
			"Key files"
		)
	);
	if(const QString filename_ = FileDialog::getInstance()->getOpenFileName(
			this,
			tr(
				"Select key file"
			),
			QString(),
			filters_
		);
		!filename_.isEmpty())
	{
		this->ui->comboKeyFile->lineEdit()->setText(
			filename_
		);
	}
}
