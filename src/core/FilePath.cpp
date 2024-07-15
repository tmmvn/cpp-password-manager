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
#include "FilePath.h"
#include <QCoreApplication>
#include <QDir>
#include <QLibrary>
#include "config-keepassx.h"
#include "core/Global.h"
FilePath* FilePath::instance(
	nullptr
);

QString FilePath::getDataPath(
	const QString &name
) const
{
	if(name.isEmpty() || name.startsWith(
		'/'
	))
	{
		return this->dataPath + name;
	}
	return this->dataPath + "/" + name;
}

QString FilePath::getPluginPath(
	const QString &name
)
{
	QStringList pluginPaths_;
    /*const QDir buildDir_(
		QCoreApplication::applicationDirPath() + "/autotype"
	);
	const QStringList buildDirEntryList_ = buildDir_.entryList(
		QDir::Dirs | QDir::NoDotAndDotDot
	);
	for(const QString &dir: buildDirEntryList_)
	{
		pluginPaths_ << QCoreApplication::applicationDirPath() + "/autotype/" +
			dir;
	}
	// for TestAutoType
	pluginPaths_ << QCoreApplication::applicationDirPath() +
        "/../src/autotype/test";*/
	pluginPaths_ << QCoreApplication::applicationDirPath();
	if(QString configuredPluginDir_ = KEEPASSX_PLUGIN_DIR;
		configuredPluginDir_ != ".")
	{
		if(QDir(
			configuredPluginDir_
		).isAbsolute())
		{
			pluginPaths_ << configuredPluginDir_;
		}
		else
		{
			const QString relativePluginDir_ = QString(
				"%1/../%2"
			).arg(
				QCoreApplication::applicationDirPath(),
				configuredPluginDir_
			);
			pluginPaths_ << QDir(
				relativePluginDir_
			).canonicalPath();
			const QString absolutePluginDir_ = QString(
				"%1/%2"
			).arg(
				KEEPASSX_PREFIX_DIR,
				configuredPluginDir_
			);
			pluginPaths_ << QDir(
				absolutePluginDir_
			).canonicalPath();
		}
	}
	QStringList dirFilter_;
	dirFilter_ << QString(
		"*%1*"
	).arg(
		name
	);
	for(const QString &path: asConst(
			pluginPaths_
		))
	{
		const QStringList fileCandidates_ = QDir(
			path
		).entryList(
			dirFilter_,
			QDir::Files
		);
		for(const QString &file: fileCandidates_)
		{
			if(QString filePath_ = path + "/" + file;
				QLibrary::isLibrary(
					filePath_
				))
			{
				return filePath_;
			}
		}
	}
	return QString();
}

QIcon FilePath::getApplicationIcon()
{
	return this->getIcon(
		"apps",
		"keepassx"
	);
}

QIcon FilePath::getIcon(
	const QString &category,
	const QString &name,
	const bool fromTheme
)
{
	QString combinedName_ = category + "/" + name;
	QIcon icon_ = this->iconCache.value(
		combinedName_
	);
	if(!icon_.isNull())
	{
		return icon_;
	}
	if(fromTheme)
	{
		icon_ = QIcon::fromTheme(
			name
		);
	}
	if(icon_.isNull())
	{
		const QList<int> pngSizes_ = {16, 22, 24, 32, 48, 64, 128};
		QString filename_;
		for(const int size_: pngSizes_)
		{
			filename_ = QString(
				"%1/icons/application/%2x%2/%3.png"
			).arg(
				this->dataPath,
				QString::number(
					size_
				),
				combinedName_
			);
			if(QFile::exists(
				filename_
			))
			{
				icon_.addFile(
					filename_,
					QSize(
						size_,
						size_
					)
				);
			}
		}
		filename_ = QString(
			"%1/icons/application/scalable/%2.svgz"
		).arg(
			this->dataPath,
			combinedName_
		);
		if(QFile::exists(
			filename_
		))
		{
			icon_.addFile(
				filename_
			);
		}
	}
	this->iconCache.insert(
		combinedName_,
		icon_
	);
	return icon_;
}

QIcon FilePath::getOnOffIcon(
	const QString &category,
	const QString &name
)
{
	QString combinedName_ = category + "/" + name;
	const QString cacheName_ = "onoff/" + combinedName_;
	QIcon icon_ = this->iconCache.value(
		cacheName_
	);
	if(!icon_.isNull())
	{
		return icon_;
	}
	for(auto i_ = 0; i_ < 2; i_++)
	{
		QIcon::State state_;
		QString stateName_;
		if(i_ == 0)
		{
			state_ = QIcon::Off;
			stateName_ = "off";
		}
		else
		{
			state_ = QIcon::On;
			stateName_ = "on";
		}
		const QList pngSizes_ = {16, 22, 24, 32, 48, 64, 128};
		QString filename_;
		for(const int size_: pngSizes_)
		{
			filename_ = QString(
				"%1/icons/application/%2x%2/%3-%4.png"
			).arg(
				this->dataPath,
				QString::number(
					size_
				),
				combinedName_,
				stateName_
			);
			if(QFile::exists(
				filename_
			))
			{
				icon_.addFile(
					filename_,
					QSize(
						size_,
						size_
					),
					QIcon::Normal,
					state_
				);
			}
		}
		filename_ = QString(
			"%1/icons/application/scalable/%2-%3.svgz"
		).arg(
			this->dataPath,
			combinedName_,
			stateName_
		);
		if(QFile::exists(
			filename_
		))
		{
			icon_.addFile(
				filename_,
				QSize(),
				QIcon::Normal,
				state_
			);
		}
	}
	this->iconCache.insert(
		cacheName_,
		icon_
	);
	return icon_;
}

FilePath::FilePath()
{
	const QString appDirPath_ = QCoreApplication::applicationDirPath();
	const bool isDataDirAbsolute_ = QDir::isAbsolutePath(
		KEEPASSX_DATA_DIR
	);
	Q_UNUSED(
		isDataDirAbsolute_
	);
	if constexpr(false)
	{
	}
#ifdef QT_DEBUG
	if(this->testSetDir(
		QString(
			KEEPASSX_SOURCE_DIR
		) + "/share"
	))
	{
	}
#endif
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    else if (this->isDataDirAbsolute && this->testSetDir(KEEPASSX_DATA_DIR)) {
    }
    else if (!this->isDataDirAbsolute && this->testSetDir(QString("%1/../%2").arg(appDirPath_, KEEPASSX_DATA_DIR))) {
    }
    else if (!this->isDataDirAbsolute && this->testSetDir(QString("%1/%2").arg(KEEPASSX_PREFIX_DIR, KEEPASSX_DATA_DIR))) {
    }
#endif
#ifdef Q_OS_MAC
	else if(this->testSetDir(
		appDirPath_ + "/../Resources"
	))
	{
	}
#endif
#ifdef Q_OS_WIN
    else if (this->testSetDir(appDirPath_ + "/share")) {
    }
#endif
	if(this->dataPath.isEmpty())
	{
		qWarning(
			"FilePath::DataPath: can't find data dir"
		);
	}
	else
	{
		this->dataPath = QDir::cleanPath(
			this->dataPath
		);
	}
}

bool FilePath::testSetDir(
	const QString &dir
)
{
	if(QFile::exists(
		dir + "/icons/database/C00_Password.png"
	))
	{
		this->dataPath = dir;
		return true;
	}
	return false;
}

FilePath* FilePath::getInstance()
{
	if(!instance)
	{
		instance = new FilePath();
	}
	return instance;
}
