/*
 *  Copyright (C) 2012 Tobias Tangemann
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
#include "Application.h"
#include <QAbstractNativeEventFilter>
#include <QFileOpenEvent>
#include "gui/entry/EditEntryWidget_p.h"
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
class XcbEventFilter : public QAbstractNativeEventFilter
{
public:
	bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) override
	{
		Q_UNUSED(result)
		Q_UNUSED(message)
		Q_UNUSED(eventType)
		return false;
	}
};
#endif
Application::Application(
	int &argc,
	char** argv
)
	: QApplication(
		argc,
		argv
	),
	mainWindow(
		nullptr
	)
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
	this->installNativeEventFilter(new XcbEventFilter());
#endif
}

void Application::setMainWindow(
	QWidget* mainWindow
)
{
	this->mainWindow = mainWindow;
}

bool Application::event(
	QEvent* event
)
{
	// Handle Apple QFileOpenEvent from finder (double click on .kdbx file)
	if(event->type() == QEvent::FileOpen)
	{
		this->sig_openFile(
			static_cast<QFileOpenEvent*>(event)->file()
		);
		return true;
	}
#ifdef Q_OS_MAC
	// restore main window when clicking on the docker icon
	if((event->type() == QEvent::ApplicationActivate) && this->mainWindow)
	{
		this->mainWindow->ensurePolished();
		this->mainWindow->setWindowState(
			this->mainWindow->windowState() & ~Qt::WindowMinimized
		);
		this->mainWindow->show();
		this->mainWindow->raise();
		this->mainWindow->activateWindow();
	}
#endif
	return QApplication::event(
		event
	);
}
