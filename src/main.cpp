/*
 *  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
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
#include <QCommandLineParser>
#include <QFile>
#include "config-keepassx.h"
#include "core/Config.h"
#include "core/Tools.h"
#include "core/Translator.h"
#include "crypto/Crypto.h"
#include "gui/Application.h"
#include "gui/MainWindow.h"
#include "gui/MessageBox.h"

int main(
	int argc,
	char** argv
)
{
#ifdef QT_NO_DEBUG
    Tools::disableCoreDumps();
#endif
	Tools::setupSearchPaths();
	Application app_(
		argc,
		argv
	);
	Application::setApplicationName(
		"keepassx"
	);
	Application::setApplicationVersion(
		KEEPASSX_VERSION
	);
	// don't set organizationName as that changes the return value of
	// QStandardPaths::writableLocation(QDesktopServices::DataLocation)
	QApplication::setQuitOnLastWindowClosed(
		false
	);
	if(!Crypto::init())
	{
		QString error_ = QCoreApplication::translate(
			"Main",
			"Fatal error while testing the cryptographic functions."
		);
		error_.append(
			"\n"
		);
		error_.append(
			Crypto::getErrorString()
		);
		MessageBox::critical(
			nullptr,
			QCoreApplication::translate(
				"Main",
				"KeePassX - Error"
			),
			error_
		);
		return 1;
	}
	QCommandLineParser parser_;
	parser_.setApplicationDescription(
		QCoreApplication::translate(
			"main",
			"KeePassX - cross-platform password manager"
		)
	);
	parser_.addPositionalArgument(
		"filename",
		QCoreApplication::translate(
			"main",
			"filename of the password database to open (*.kdbx)"
		)
	);
	const QCommandLineOption configOption_(
		"config",
		QCoreApplication::translate(
			"main",
			"path to a custom config file"
		),
		"config"
	);
	const QCommandLineOption keyfileOption_(
		"keyfile",
		QCoreApplication::translate(
			"main",
			"key file of the database"
		),
		"keyfile"
	);
	parser_.addHelpOption();
	parser_.addVersionOption();
	parser_.addOption(
		configOption_
	);
	parser_.addOption(
		keyfileOption_
	);
	parser_.process(
		app_
	);
	const QStringList args_ = parser_.positionalArguments();
	if(parser_.isSet(
		configOption_
	))
	{
		Config::createConfigFromFile(
			parser_.value(
				configOption_
			)
		);
	}
	Translator::installTranslator();
#ifdef Q_OS_MAC
	// Don't show menu icons on OSX
	QApplication::setAttribute(
		Qt::AA_DontShowIconsInMenus
	);
#endif
	MainWindow mainWindow_;
	mainWindow_.show();
	app_.setMainWindow(
		&mainWindow_
	);
	QObject::connect(
		&app_,
		&Application::sig_openFile,
		&mainWindow_,
		&MainWindow::do_openDatabase
	);
	if(!args_.isEmpty())
	{
		if(const QString &filename_ = args_[0];
			!filename_.isEmpty() && QFile::exists(
				filename_
			))
		{
			mainWindow_.openDatabase(
				filename_,
				QString(),
				parser_.value(
					keyfileOption_
				)
			);
		}
	}
	if(Config::getInstance()->get(
		"OpenPreviousDatabasesOnStartup"
	).toBool())
	{
		const QStringList filenames_ = Config::getInstance()->get(
			"LastOpenedDatabases"
		).toStringList();
		for(const QString &filename_: filenames_)
		{
			if(!filename_.isEmpty() && QFile::exists(
				filename_
			))
			{
				mainWindow_.openDatabase(
					filename_,
					QString(),
					QString()
				);
			}
		}
	}
	return Application::exec();
}
