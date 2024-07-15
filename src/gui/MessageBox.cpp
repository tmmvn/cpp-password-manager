/*
 *  Copyright (C) 2013 Felix Geyer <debfx@fobos.de>
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
#include "MessageBox.h"
QMessageBox::StandardButton MessageBox::nextAnswer(
	QMessageBox::NoButton
);

QMessageBox::StandardButton MessageBox::critical(
	QWidget* parent,
	const QString &title,
	const QString &text,
	const QMessageBox::StandardButtons buttons,
	const QMessageBox::StandardButton defaultButton
)
{
	if(nextAnswer == QMessageBox::NoButton)
	{
		return QMessageBox::critical(
			parent,
			title,
			text,
			buttons,
			defaultButton
		);
	}
	const QMessageBox::StandardButton returnButton_ = nextAnswer;
	nextAnswer = QMessageBox::NoButton;
	return returnButton_;
}

QMessageBox::StandardButton MessageBox::information(
	QWidget* parent,
	const QString &title,
	const QString &text,
	const QMessageBox::StandardButtons buttons,
	const QMessageBox::StandardButton defaultButton
)
{
	if(nextAnswer == QMessageBox::NoButton)
	{
		return QMessageBox::information(
			parent,
			title,
			text,
			buttons,
			defaultButton
		);
	}
	const QMessageBox::StandardButton returnButton_ = nextAnswer;
	nextAnswer = QMessageBox::NoButton;
	return returnButton_;
}

QMessageBox::StandardButton MessageBox::question(
	QWidget* parent,
	const QString &title,
	const QString &text,
	const QMessageBox::StandardButtons buttons,
	const QMessageBox::StandardButton defaultButton
)
{
	if(nextAnswer == QMessageBox::NoButton)
	{
		return QMessageBox::question(
			parent,
			title,
			text,
			buttons,
			defaultButton
		);
	}
	const QMessageBox::StandardButton returnButton_ = nextAnswer;
	nextAnswer = QMessageBox::NoButton;
	return returnButton_;
}

QMessageBox::StandardButton MessageBox::warning(
	QWidget* parent,
	const QString &title,
	const QString &text,
	const QMessageBox::StandardButtons buttons,
	const QMessageBox::StandardButton defaultButton
)
{
	if(nextAnswer == QMessageBox::NoButton)
	{
		return QMessageBox::warning(
			parent,
			title,
			text,
			buttons,
			defaultButton
		);
	}
	const QMessageBox::StandardButton returnButton_ = nextAnswer;
	nextAnswer = QMessageBox::NoButton;
	return returnButton_;
}

void MessageBox::setNextAnswer(
	const QMessageBox::StandardButton button
)
{
	nextAnswer = button;
}
