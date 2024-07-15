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
#include "InactivityTimer.h"
#include <QCoreApplication>
#include <QTimer>

InactivityTimer::InactivityTimer(
	QObject* parent
)
	: QObject(
		parent
	),
	timer(
		new QTimer(
			this
		)
	),
	active(
		false
	)
{
	this->timer->setSingleShot(
		true
	);
	this->connect(
		this->timer,
		&QTimer::timeout,
		this,
		&InactivityTimer::do_timeout
	);
}

void InactivityTimer::setInactivityTimeout(
	const int inactivityTimeout
) const
{
	if(inactivityTimeout <= 0)
	{
		return;
	}
	this->timer->setInterval(
		inactivityTimeout
	);
}

void InactivityTimer::activate()
{
	if(!this->active)
	{
		qApp->installEventFilter(
			this
		);
	}
	this->active = true;
	this->timer->start();
}

void InactivityTimer::deactivate()
{
	qApp->removeEventFilter(
		this
	);
	this->active = false;
	this->timer->stop();
}

bool InactivityTimer::eventFilter(
	QObject* watched,
	QEvent* event
)
{
	if(const QEvent::Type type_ = event->type();
		(type_ >= QEvent::MouseButtonPress && type_ <= QEvent::KeyRelease) || (
			type_ >= QEvent::HoverEnter && type_ <= QEvent::HoverMove) || (type_
			== QEvent::Wheel))
	{
		this->timer->start();
	}
	return QObject::eventFilter(
		watched,
		event
	);
}

void InactivityTimer::do_timeout()
{
	// make sure we don't emit the signal a second time while it's still processed
	if(!this->emitMutx.tryLock())
	{
		return;
	}
	if(this->active && !this->timer->isActive())
	{
		 sig_inactivityDetected();
	}
	this->emitMutx.unlock();
}
