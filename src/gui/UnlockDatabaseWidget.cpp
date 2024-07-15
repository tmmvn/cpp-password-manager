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
#include "UnlockDatabaseWidget.h"
#include "ui_DatabaseOpenWidget.h"
#include "core/Database.h"

UnlockDatabaseWidget::UnlockDatabaseWidget(
	QWidget* parent
)
	: DatabaseOpenWidget(
		parent
	)
{
	this->ui->labelHeadline->setText(
		this->tr(
			"Unlock database"
		)
	);
}

void UnlockDatabaseWidget::clearForms()
{
	this->ui->editPassword->clear();
	this->ui->comboKeyFile->clear();
	this->ui->checkPassword->setChecked(
		false
	);
	this->ui->checkKeyFile->setChecked(
		false
	);
	this->ui->buttonTogglePassword->setChecked(
		false
	);
	this->db = nullptr;
}
