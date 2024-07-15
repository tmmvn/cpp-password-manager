/*
 *  Copyright (C) 2007 Trolltech ASA <info@trolltech.com>
 *  Copyright (C) 2012 Felix Geyer <debfx@fobos.de>
 *  Copyright (C) 2012 Florian Geyer <blueice@fobos.de>
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
#include "LineEdit.h"
#include <QStyle>
#include <QToolButton>
#include "core/FilePath.h"

LineEdit::LineEdit(
	QWidget* parent
)
	: QLineEdit(
		parent
	),
	clearButton(
		new QToolButton(
			this
		)
	)
{
	this->clearButton->setObjectName(
		"clearButton"
	);
	const QString iconNameDirected_ = QString(
		"edit-clear-locationbar-"
	).append(
		(layoutDirection() == Qt::LeftToRight) ? "rtl" : "ltr"
	);
	QIcon icon_ = QIcon::fromTheme(
		iconNameDirected_
	);
	if(icon_.isNull())
	{
		icon_ = QIcon::fromTheme(
			"edit-clear"
		);
		if(icon_.isNull())
		{
			icon_ = FilePath::getInstance()->getIcon(
				"actions",
				iconNameDirected_,
				false
			);
		}
	}
	this->clearButton->setIcon(
		icon_
	);
	this->clearButton->setCursor(
		Qt::ArrowCursor
	);
	this->clearButton->setStyleSheet(
		"QToolButton { border: none; padding: 0px; }"
	);
	this->clearButton->hide();
	this->connect(
		this->clearButton,
		&QToolButton::clicked,
		this,
		&LineEdit::clear
	);
	this->connect(
		this,
		&LineEdit::textChanged,
		this,
		&LineEdit::do_updateCloseButton
	);
	const int frameWidth_ = this->style()->pixelMetric(
		QStyle::PM_DefaultFrameWidth
	);
	this->setStyleSheet(
		QString(
			"QLineEdit { padding-right: %1px; } "
		).arg(
			this->clearButton->sizeHint().width() + frameWidth_ + 1
		)
	);
	const QSize msz_ = QLineEdit::minimumSizeHint();
	this->setMinimumSize(
		qMax(
			msz_.width(),
			this->clearButton->sizeHint().height() + frameWidth_ * 2 + 2
		),
		qMax(
			msz_.height(),
			this->clearButton->sizeHint().height() + frameWidth_ * 2 + 2
		)
	);
}

void LineEdit::resizeEvent(
	QResizeEvent* event
)
{
	const QSize sz_ = this->clearButton->sizeHint();
	const int frameWidth_ = this->style()->pixelMetric(
		QStyle::PM_DefaultFrameWidth
	);
	const int y_ = (this->rect().bottom() + 1 - sz_.height()) / 2;
	if(this->layoutDirection() == Qt::LeftToRight)
	{
		this->clearButton->move(
			rect().right() - frameWidth_ - sz_.width(),
			y_
		);
	}
	else
	{
		this->clearButton->move(
			rect().left() + frameWidth_,
			y_
		);
	}
	QLineEdit::resizeEvent(
		event
	);
}

void LineEdit::do_updateCloseButton(
	const QString &text
) const
{
	this->clearButton->setVisible(
		!text.isEmpty()
	);
}
