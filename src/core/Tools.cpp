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
#include "Tools.h"
#include <QImageReader>
#include <QIODevice>
#include <QLocale>
#include <QStringList>
#ifdef Q_OS_WIN
#include <windows.h> // for Sleep(), SetDllDirectoryA() and SetSearchPathMode()
#endif
#ifdef Q_OS_UNIX
#include <time.h> // for nanosleep()
#endif
#include "config-keepassx.h"
#if defined(HAVE_RLIMIT_CORE)
#include <sys/resource.h>
#endif
#if defined(HAVE_PR_SET_DUMPABLE)
#include <sys/prctl.h>
#endif
#ifdef HAVE_PT_DENY_ATTACH
#include <sys/ptrace.h>
#include <sys/types.h>
#endif
namespace Tools
{
	QString humanReadableFileSize(
		const qint64 bytes
	)
	{
		qint64 size_ = bytes;
		const QStringList units_ = QStringList() << "B" << "KiB" << "MiB" <<
			"GiB";
		auto i_ = 0;
		const qsizetype maxI_ = units_.size() - 1;
		while((size_ >= 1024) && (i_ < maxI_))
		{
			size_ /= 1024;
			i_++;
		}
		return QString(
			"%1 %2"
		).arg(
			QLocale().toString(
				static_cast<double>(size_),
				'f',
				2
			),
			units_.at(
				i_
			)
		);
	}

	bool hasChild(
		const QObject* parent,
		const QObject* child
	)
	{
		if(!parent || !child)
		{
			return false;
		}
		const QObjectList children_ = parent->children();
		for(const QObject* c_: children_)
		{
			if(child == c_ || hasChild(
				c_,
				child
			))
			{
				return true;
			}
		}
		return false;
	}

	bool readFromDevice(
		QIODevice* device,
		QByteArray &data,
		const int size
	)
	{
		QByteArray buffer_;
		buffer_.resize(
			size
		);
		const qint64 readResult_ = device->read(
			buffer_.data(),
			size
		);
		if(readResult_ == -1)
		{
			return false;
		}
		buffer_.resize(
			readResult_
		);
		data = buffer_;
		return true;
	}

	bool readAllFromDevice(
		QIODevice* device,
		QByteArray &data
	)
	{
		QByteArray result_;
		qint64 readBytes_ = 0;
		qint64 readResult_;
		do
		{
			result_.resize(
				result_.size() + 16384
			);
			readResult_ = device->read(
				result_.data() + readBytes_,
				result_.size() - readBytes_
			);
			if(readResult_ > 0)
			{
				readBytes_ += readResult_;
			}
		}
		while(readResult_ > 0);
		if(readResult_ == -1)
		{
			return false;
		}
		result_.resize(
			static_cast<int>(readBytes_)
		);
		data = result_;
		return true;
	}

	QString getImageReaderFilter()
	{
		const QList<QByteArray> formats_ =
			QImageReader::supportedImageFormats();
		QStringList formatsStringList_;
		for(const QByteArray &format_: formats_)
		{
			// TODO: This loop logic doesn't make sense
			for(auto i_ = 0; i_ < format_.size(); i_++)
			{
				if(!QChar(
					format_.at(
						i_
					)
				).isLetterOrNumber())
				{
					continue;
				}
			}
			formatsStringList_.append(
				"*." + QString::fromLatin1(
					format_
				).toLower()
			);
		}
		return formatsStringList_.join(
			" "
		);
	}

	bool isHex(
		const QByteArray &ba
	)
	{
		for(const char c_: ba)
		{
			if(!((c_ >= '0' && c_ <= '9') || (c_ >= 'a' && c_ <= 'f') || (c_ >=
				'A' && c_ <= 'F')))
			{
				return false;
			}
		}
		return true;
	}

	bool isBase64(
		const QByteArray &ba
	)
	{
		const QRegularExpression regexp_(
			"^(?:[a-z0-9+/]{4})*(?:[a-z0-9+/]{3}=|[a-z0-9+/]{2}==)?$",
			QRegularExpression::CaseInsensitiveOption
		);
		const QString base64_ = QString::fromLatin1(
			ba.constData(),
			ba.size()
		);
		return regexp_.match(
			base64_
		).hasMatch();
	}

	void sleep(
		int ms
	)
	{
		if(ms == 0)
		{
			return;
		}
#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
		timespec ts_;
		ts_.tv_sec = ms / 1000;
		ts_.tv_nsec = (static_cast<long>(ms) % 1000) * 1000 * 1000;
		nanosleep(
			&ts_,
			nullptr
		);
#endif
	}

	void wait(
		const int ms
	)
	{
		if(ms == 0)
		{
			return;
		}
		QElapsedTimer timer_;
		timer_.start();
		if(ms <= 50)
		{
			QCoreApplication::processEvents(
				QEventLoop::AllEvents,
				ms
			);
			sleep(
				qMax(
					ms - static_cast<int>(timer_.elapsed()),
					0
				)
			);
		}
		else
		{
			do
			{
				if(const int timeLeft_ = ms - static_cast<int>(timer_.elapsed())
					;
					timeLeft_ > 0)
				{
					QCoreApplication::processEvents(
						QEventLoop::AllEvents,
						timeLeft_
					);
					sleep(
						10
					);
				}
			}
			while(!timer_.hasExpired(
				ms
			));
		}
	}

	void disableCoreDumps()
	{
		// default to true
		// there is no point in printing a warning if this is not implemented on the platform
		bool success_;
#if defined(HAVE_RLIMIT_CORE)
		rlimit limit_;
		limit_.rlim_cur = 0;
		limit_.rlim_max = 0;
		success_ = setrlimit(
			RLIMIT_CORE,
			&limit_
		) == 0;
#endif
#if defined(HAVE_PR_SET_DUMPABLE)
    success_ = success_ && (prctl(PR_SET_DUMPABLE, 0) == 0);
#endif
		// Mac OS X
#ifdef HAVE_PT_DENY_ATTACH
		success_ = success_ && (ptrace(
			PT_DENY_ATTACH,
			0,
			nullptr,
			0
		) == 0);
#endif
		if(!success_)
		{
			qWarning(
				"Unable to disable core dumps."
			);
		}
	}

	void setupSearchPaths()
	{
#ifdef Q_OS_WIN
    // Make sure Windows doesn't load DLLs from the current working directory
    SetDllDirectoryA("");
    SetSearchPathMode(BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE);
#endif
	}
} // namespace Tools
