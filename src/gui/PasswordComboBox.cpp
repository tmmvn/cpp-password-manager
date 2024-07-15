/*
 *  Copyright (C) 2013 Michael Curtis <michael@moltenmercury.org>
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
#include "PasswordComboBox.h"
#include <QLineEdit>
#include "core/PasswordGenerator.h"

PasswordComboBox::PasswordComboBox(
	QWidget* parent
)
	: QComboBox(
		parent
	),
	generator(
		nullptr
	),
	alternatives(
		10
	)
{
	this->setEditable(
		true
	);
	this->do_setEcho(
		false
	);
}

PasswordComboBox::~PasswordComboBox()
{
}

void PasswordComboBox::do_setEcho(
	const bool echo
)
{
	this->lineEdit()->setEchoMode(
		echo ? QLineEdit::Normal : QLineEdit::Password
	);
	const QString current_ = this->currentText();
	if(echo)
	{
		// add fake item to show visual indication that a popup is available
		this->addItem(
			""
		);
#ifdef Q_OS_MAC
		// Qt on Mac OS doesn't seem to know the generic monospace family (tested with 4.8.6)
		this->setStyleSheet(
			"QComboBox { font-family: monospace,Menlo,Monaco; }"
		);
#else
        this->setStyleSheet("QComboBox { font-family: monospace,Courier New; }");
#endif
	}
	else
	{
		// clear items so the combobox indicates that no popup menu is available
		this->clear();
		this->setStyleSheet(
			"QComboBox { font-family: initial; }"
		);
	}
	this->setEditText(
		current_
	);
}

void PasswordComboBox::setGenerator(
	PasswordGenerator* generator
)
{
	this->generator = generator;
}

void PasswordComboBox::setNumberAlternatives(
	const int alternatives
)
{
	this->alternatives = alternatives;
}

void PasswordComboBox::showPopup()
{
	// no point in showing a bunch of hidden passwords
	if(this->lineEdit()->echoMode() == QLineEdit::Password)
	{
		this->hidePopup();
		return;
	}
	// keep existing password as the first item in the popup
	const QString current_ = this->currentText();
	this->clear();
	this->addItem(
		current_
	);
	if(this->generator && this->generator->isValid())
	{
		for(auto alternative_ = 0; alternative_ < this->alternatives;
			alternative_++)
		{
			QString password_ = this->generator->generatePassword();
			this->addItem(
				password_
			);
		}
	}
	QComboBox::showPopup();
}
