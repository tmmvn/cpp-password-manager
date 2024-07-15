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
#include "DialogWidget.h"
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QPushButton>

DialogWidget::DialogWidget(
	QWidget* parent
)
	: QWidget(
		parent
	)
{
}

void DialogWidget::keyPressEvent(
	QKeyEvent* e
)
{
#ifdef Q_OS_MAC
	if(e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period)
	{
		if(!this->clickButton(
			QDialogButtonBox::Cancel
		))
		{
			e->ignore();
		}
	}
	else
#endif
		if(!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key()
			== Qt::Key_Enter))
		{
			switch(e->key())
			{
				case Qt::Key_Enter:
				case Qt::Key_Return:
					if(!this->clickButton(
						QDialogButtonBox::Ok
					))
					{
						e->ignore();
					}
					break;
				case Qt::Key_Escape:
					if(!this->clickButton(
						QDialogButtonBox::Cancel
					))
					{
						if(!this->clickButton(
							QDialogButtonBox::Close
						))
						{
							e->ignore();
						}
					}
					break;
				default:
					e->ignore();
			}
		}
		else
		{
			e->ignore();
		}
}

bool DialogWidget::clickButton(
	const QDialogButtonBox::StandardButton standardButton
) const
{
	QPushButton* pb_;
	if(standardButton == QDialogButtonBox::Ok)
	{
		pb_ = qobject_cast<QPushButton*>(
			this->focusWidget()
		);
		if(pb_ && pb_->isVisible() && pb_->isEnabled() && pb_->hasFocus())
		{
			pb_->click();
			return true;
		}
	}
	const QList<QDialogButtonBox*> buttonBoxes_ = this->findChildren<
		QDialogButtonBox*>();
	for(auto i = 0; i < buttonBoxes_.size(); ++i)
	{
		const QDialogButtonBox* buttonBox_ = buttonBoxes_.at(
			i
		);
		pb_ = buttonBox_->button(
			standardButton
		);
		if(pb_ && pb_->isVisible() && pb_->isEnabled())
		{
			pb_->click();
			return true;
		}
	}
	return false;
}
