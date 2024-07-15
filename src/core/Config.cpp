/*
 *  Copyright (C) 2011 Felix Geyer <debfx@fobos.de>
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
#include "Config.h"
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryFile>
Config* Config::instance(
	nullptr
);

QVariant Config::get(
	const QString &key
) const
{
	return this->settings->value(
		key,
		this->defaults.value(
			key
		)
	);
}

QVariant Config::get(
	const QString &key,
	const QVariant &defaultValue
) const
{
	return this->settings->value(
		key,
		defaultValue
	);
}

void Config::set(
	const QString &key,
	const QVariant &value
) const
{
	this->settings->setValue(
		key,
		value
	);
}

Config::Config(
	const QString &fileName,
	QObject* parent
)
	: QObject(
		parent
	)
{
	this->init(
		fileName
	);
}

Config::Config(
	QObject* parent
)
	: QObject(
		parent
	)
{
	QString userPath_;
	QString homePath_ = QDir::homePath();
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    // we can't use QStandardPaths on X11 as it uses XDG_DATA_HOME instead of XDG_CONFIG_HOME
    QByteArray env_ = qgetenv("XDG_CONFIG_HOME");
    if (env_.isEmpty()) {
        userPath_ = homePath;
        userPath_ += "/.config";
    }
    else if (env_[0] == '/') {
        userPath_ = QFile::decodeName(env_);
    }
    else {
        userPath_ = homePath;
        userPath_ += '/';
        userPath_ += QFile::decodeName(env_);
    }
    userPath_ += "/keepassx/";
#else
	userPath_ = QDir::fromNativeSeparators(
		QStandardPaths::writableLocation(
			QStandardPaths::StateLocation
		)
	);
	// storageLocation() appends the application name ("/keepassx") to the end
	userPath_ += "/";
#endif
	userPath_ += "keepassx2.ini";
	this->init(
		userPath_
	);
}

Config::~Config()
{
}

void Config::init(
	const QString &fileName
)
{
	this->settings.reset(
		new QSettings(
			fileName,
			QSettings::IniFormat
		)
	);
	this->defaults.insert(
		"RememberLastDatabases",
		true
	);
	this->defaults.insert(
		"RememberLastKeyFiles",
		true
	);
	this->defaults.insert(
		"OpenPreviousDatabasesOnStartup",
		true
	);
	this->defaults.insert(
		"AutoSaveAfterEveryChange",
		false
	);
	this->defaults.insert(
		"AutoSaveOnExit",
		false
	);
	this->defaults.insert(
		"ShowToolbar",
		true
	);
	this->defaults.insert(
		"MinimizeOnCopy",
		false
	);
	this->defaults.insert(
		"UseGroupIconOnEntryCreation",
		false
	);
	this->defaults.insert(
		"security/clearclipboard",
		true
	);
	this->defaults.insert(
		"security/clearclipboardtimeout",
		10
	);
	this->defaults.insert(
		"security/lockdatabaseidle",
		false
	);
	this->defaults.insert(
		"security/lockdatabaseidlesec",
		10
	);
	this->defaults.insert(
		"security/passwordscleartext",
		false
	);
	this->defaults.insert(
		"GUI/Language",
		"system"
	);
	this->defaults.insert(
		"GUI/ShowTrayIcon",
		false
	);
	this->defaults.insert(
		"GUI/MinimizeToTray",
		false
	);
}

Config* Config::getInstance()
{
	if(!instance)
	{
		instance = new Config(
			qApp
		);
	}
	return instance;
}

void Config::createConfigFromFile(
	const QString &file
)
{
	if(instance)
	{
		return;
	}
	instance = new Config(
		file,
		qApp
	);
}

void Config::createTempFileInstance()
{
	if(instance)
	{
		return;
	}
	const auto tmpFile_ = new QTemporaryFile();
	const bool openResult_ = tmpFile_->open();
	if(!openResult_)
	{
		return;
	}
	Q_UNUSED(
		openResult_
	);
	instance = new Config(
		tmpFile_->fileName(),
		qApp
	);
	tmpFile_->setParent(
		instance
	);
}
