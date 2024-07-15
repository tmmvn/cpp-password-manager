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
#include "ChangeMasterKeyWidget.h"
#include "ui_ChangeMasterKeyWidget.h"
#include "core/FilePath.h"
#include "gui/FileDialog.h"
#include "gui/MessageBox.h"
#include "keys/FileKey.h"
#include "keys/PasswordKey.h"

ChangeMasterKeyWidget::ChangeMasterKeyWidget(
	QWidget* parent
)
	: DialogWidget(
		parent
	),
	ui(
		new Ui::ChangeMasterKeyWidget()
	)
{
	this->ui->setupUi(
		this
	);
	this->connect(
		this->ui->buttonBox,
		&QDialogButtonBox::accepted,
		this,
		&ChangeMasterKeyWidget::do_generateKey
	);
	this->connect(
		this->ui->buttonBox,
		&QDialogButtonBox::rejected,
		this,
		&ChangeMasterKeyWidget::do_reject
	);
	this->ui->togglePasswordButton->setIcon(
		FilePath::getInstance()->getOnOffIcon(
			"actions",
			"password-show"
		)
	);
	this->connect(
		this->ui->togglePasswordButton,
		&QToolButton::toggled,
		this->ui->enterPasswordEdit,
		&PasswordEdit::do_setShowPassword
	);
	this->ui->repeatPasswordEdit->enableVerifyMode(
		this->ui->enterPasswordEdit
	);
	this->connect(
		this->ui->createKeyFileButton,
		&QPushButton::clicked,
		this,
		&ChangeMasterKeyWidget::do_createKeyFile
	);
	this->connect(
		this->ui->browseKeyFileButton,
		&QPushButton::clicked,
		this,
		&ChangeMasterKeyWidget::do_browseKeyFile
	);
}

ChangeMasterKeyWidget::~ChangeMasterKeyWidget()
{
}

void ChangeMasterKeyWidget::do_createKeyFile()
{
	const QString filters_ = QString(
		"%1 (*.key);;%2 (*)"
	).arg(
		tr(
			"Key files"
		),
		tr(
			"All files"
		)
	);
	if(const QString fileName_ = FileDialog::getInstance()->getSaveFileName(
			this,
			tr(
				"Create Key File..."
			),
			QString(),
			filters_
		);
		!fileName_.isEmpty())
	{
		QString errorMsg_;
		if(const bool created_ = FileKey::create(
				fileName_,
				&errorMsg_
			);
			!created_)
		{
			MessageBox::warning(
				this,
				tr(
					"Error"
				),
				tr(
					"Unable to create Key File : "
				) + errorMsg_
			);
		}
		else
		{
			this->ui->keyFileCombo->setEditText(
				fileName_
			);
		}
	}
}

void ChangeMasterKeyWidget::do_browseKeyFile()
{
	const QString filters_ = QString(
		"%1 (*.key);;%2 (*)"
	).arg(
		tr(
			"Key files"
		),
		tr(
			"All files"
		)
	);
	if(const QString fileName_ = FileDialog::getInstance()->getOpenFileName(
			this,
			tr(
				"Select a key file"
			),
			QString(),
			filters_
		);
		!fileName_.isEmpty())
	{
		this->ui->keyFileCombo->setEditText(
			fileName_
		);
	}
}

void ChangeMasterKeyWidget::clearForms()
{
	this->key.clear();
	this->ui->passwordGroup->setChecked(
		true
	);
	this->ui->enterPasswordEdit->setText(
		""
	);
	this->ui->repeatPasswordEdit->setText(
		""
	);
	this->ui->keyFileGroup->setChecked(
		false
	);
	this->ui->togglePasswordButton->setChecked(
		false
	);
	// TODO: clear m_ui->keyFileCombo
	this->ui->enterPasswordEdit->setFocus();
}

CompositeKey ChangeMasterKeyWidget::newMasterKey()
{
	return this->key;
}

QLabel* ChangeMasterKeyWidget::headlineLabel() const
{
	return this->ui->headlineLabel;
}

void ChangeMasterKeyWidget::do_generateKey()
{
	this->key.clear();
	if(this->ui->passwordGroup->isChecked())
	{
		if(this->ui->enterPasswordEdit->text() == this->ui->repeatPasswordEdit->
			text())
		{
			if(this->ui->enterPasswordEdit->text().isEmpty())
			{
				if(MessageBox::question(
					this,
					tr(
						"Question"
					),
					tr(
						"Do you really want to use an empty string as password?"
					),
					QMessageBox::Yes | QMessageBox::No
				) != QMessageBox::Yes)
				{
					return;
				}
			}
			this->key.addKey(
				PasswordKey(
					this->ui->enterPasswordEdit->text()
				)
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
					"Different passwords supplied."
				)
			);
			this->ui->enterPasswordEdit->setText(
				""
			);
			this->ui->repeatPasswordEdit->setText(
				""
			);
			return;
		}
	}
	if(this->ui->keyFileGroup->isChecked())
	{
		FileKey fileKey_;
		QString errorMsg_;
		if(!fileKey_.load(
			this->ui->keyFileCombo->currentText(),
			&errorMsg_
		))
		{
			MessageBox::critical(
				this,
				tr(
					"Failed to set key file"
				),
				tr(
					"Failed to set %1 as the Key file:\n%2"
				).arg(
					this->ui->keyFileCombo->currentText(),
					errorMsg_
				)
			);
			return;
		}
		this->key.addKey(
			fileKey_
		);
	}
	 this->sig_editFinished(
		true
	);
}

void ChangeMasterKeyWidget::do_reject()
{
	 this->sig_editFinished(
		false
	);
}
