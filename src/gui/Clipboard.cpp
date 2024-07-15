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
#include "Clipboard.h"
#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include "core/Config.h"
Clipboard* Clipboard::instance(
	nullptr
);

Clipboard::Clipboard(
	QObject* parent
)
	: QObject(
		parent
	),
	timer(
		new QTimer(
			this
		)
	)
{
	this->timer->setSingleShot(
		true
	);
	this->connect(
		this->timer,
		&QTimer::timeout,
		this,
		&Clipboard::do_clearClipboard
	);
	this->connect(
		qApp,
		&QApplication::aboutToQuit,
		this,
		&Clipboard::do_clearCopiedText
	);
}

void Clipboard::setText(
	const QString &text
)
{
	QClipboard* clipboard_ = QApplication::clipboard();
	clipboard_->setText(
		text,
		QClipboard::Clipboard
	);
	if(clipboard_->supportsSelection())
	{
		clipboard_->setText(
			text,
			QClipboard::Selection
		);
	}
	if(Config::getInstance()->get(
		"security/clearclipboard"
	).toBool())
	{
		if(const int timeout_ = Config::getInstance()->get(
				"security/clearclipboardtimeout"
			).toInt();
			timeout_ > 0)
		{
			this->lastCopied = text;
			this->timer->start(
				timeout_ * 1000
			);
		}
	}
}

void Clipboard::do_clearCopiedText()
{
	if(this->timer->isActive())
	{
		this->timer->stop();
		this->do_clearClipboard();
	}
}

void Clipboard::do_clearClipboard()
{
	QClipboard* clipboard_ = QApplication::clipboard();
	if(!clipboard_)
	{
		qWarning(
			"Unable to access the clipboard."
		);
		return;
	}
	if(clipboard_->text(
		QClipboard::Clipboard
	) == this->lastCopied)
	{
		clipboard_->clear(
			QClipboard::Clipboard
		);
	}
	if(clipboard_->supportsSelection() && (clipboard_->text(
		QClipboard::Selection
	) == this->lastCopied))
	{
		clipboard_->clear(
			QClipboard::Selection
		);
	}
	this->lastCopied.clear();
}

Clipboard* Clipboard::getInstance()
{
	if(!instance)
	{
		instance = new Clipboard(
			qApp
		);
	}
	return instance;
}
