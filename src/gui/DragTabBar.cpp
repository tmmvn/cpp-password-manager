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
#include "DragTabBar.h"
#include <QApplication>
#include <QDragEnterEvent>
#include <QTimer>

DragTabBar::DragTabBar(
	QWidget* parent
)
	: QTabBar(
		parent
	),
	tabSwitchTimer(
		new QTimer(
			this
		)
	),
	tabSwitchIndex(
		-1
	)
{
	this->tabSwitchTimer->setSingleShot(
		true
	);
	this->connect(
		this->tabSwitchTimer,
		&QTimer::timeout,
		this,
		&DragTabBar::do_dragSwitchTab
	);
	this->setAcceptDrops(
		true
	);
}

void DragTabBar::dragEnterEvent(
	QDragEnterEvent* event
)
{
	if(const int tab_ = this->tabAt(
			event->position().toPoint()
		);
		tab_ != -1)
	{
		if(tab_ != this->currentIndex())
		{
			this->tabSwitchIndex = tab_;
			this->tabSwitchTimer->start(
				QApplication::doubleClickInterval() * 2
			);
		}
		event->setAccepted(
			true
		);
	}
	else
	{
		QTabBar::dragEnterEvent(
			event
		);
	}
}

void DragTabBar::dragMoveEvent(
	QDragMoveEvent* event
)
{
	if(const int tab_ = this->tabAt(
			event->position().toPoint()
		);
		tab_ != -1)
	{
		if(tab_ == this->currentIndex())
		{
			this->tabSwitchTimer->stop();
		}
		else if(tab_ != this->tabSwitchIndex)
		{
			this->tabSwitchIndex = tab_;
			this->tabSwitchTimer->start(
				QApplication::doubleClickInterval() * 2
			);
		}
		event->setAccepted(
			true
		);
	}
	else
	{
		this->tabSwitchIndex = -1;
		this->tabSwitchTimer->stop();
		QTabBar::dragMoveEvent(
			event
		);
	}
}

void DragTabBar::dragLeaveEvent(
	QDragLeaveEvent* event
)
{
	this->tabSwitchIndex = -1;
	this->tabSwitchTimer->stop();
	QTabBar::dragLeaveEvent(
		event
	);
}

void DragTabBar::dropEvent(
	QDropEvent* event
)
{
	this->tabSwitchIndex = -1;
	this->tabSwitchTimer->stop();
	QTabBar::dropEvent(
		event
	);
}

void DragTabBar::tabLayoutChange()
{
	this->tabSwitchIndex = -1;
	this->tabSwitchTimer->stop();
	QTabBar::tabLayoutChange();
}

void DragTabBar::do_dragSwitchTab()
{
	if(const int tab_ = this->tabAt(
			this->mapFromGlobal(
				QCursor::pos()
			)
		);
		tab_ != -1 && tab_ == this->tabSwitchIndex)
	{
		this->tabSwitchIndex = -1;
		this->setCurrentIndex(
			tab_
		);
	}
}
