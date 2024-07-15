/*
 *  Copyright (C) 2014 Felix Geyer <debfx@fobos.de>
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
#include "PasswordEdit.h"
const QColor PasswordEdit::CorrectSoFarColor = QColor(
	255,
	205,
	15
);
const QColor PasswordEdit::ErrorColor = QColor(
	255,
	125,
	125
);

PasswordEdit::PasswordEdit(
	QWidget* parent
)
	: QLineEdit(
		parent
	),
	basePasswordEdit(
		nullptr
	)
{
}

void PasswordEdit::enableVerifyMode(
	PasswordEdit* baseEdit
)
{
	this->basePasswordEdit = baseEdit;
	this->do_updateStylesheet();
	this->connect(
		this->basePasswordEdit,
		&PasswordEdit::textChanged,
		this,
		&PasswordEdit::do_updateStylesheet
	);
	this->connect(
		this,
		&PasswordEdit::textChanged,
		this,
		&PasswordEdit::do_updateStylesheet
	);
	this->connect(
		this->basePasswordEdit,
		&PasswordEdit::sig_showPasswordChanged,
		this,
		&PasswordEdit::do_setShowPassword
	);
}

void PasswordEdit::do_setShowPassword(
	const bool show
)
{
	this->setEchoMode(
		show ? this->Normal : this->Password
	);
	this->do_updateStylesheet();
	 this->sig_showPasswordChanged(
		show
	);
}

bool PasswordEdit::passwordsEqual() const
{
	return this->text() == this->basePasswordEdit->text();
}

void PasswordEdit::do_updateStylesheet()
{
	QString stylesheet_(
		"QLineEdit { "
	);
	if(this->echoMode() == this->Normal)
	{
#ifdef Q_OS_MAC
		// Qt on Mac OS doesn't seem to know the generic monospace family (tested with 4.8.6)
		stylesheet_.append(
			"font-family: monospace,Menlo,Monaco; "
		);
#else
        stylesheet_.append("font-family: monospace,Courier New; ");
#endif
	}
	if(this->basePasswordEdit && !this->passwordsEqual())
	{
		stylesheet_.append(
			"background: %1; "
		);
		if(this->basePasswordEdit->text().startsWith(
			this->text()
		))
		{
			stylesheet_ = stylesheet_.arg(
				this->CorrectSoFarColor.name()
			);
		}
		else
		{
			stylesheet_ = stylesheet_.arg(
				this->ErrorColor.name()
			);
		}
	}
	stylesheet_.append(
		"}"
	);
	this->setStyleSheet(
		stylesheet_
	);
}
