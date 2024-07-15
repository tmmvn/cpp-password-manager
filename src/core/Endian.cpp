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
#include "Endian.h"
#include <QIODevice>
#include <QtEndian>

namespace Endian
{
	qint16 bytesToInt16(
		const QByteArray &ba,
		const QSysInfo::Endian byteOrder
	)
	{
		if(ba.size() != 2)
		{
			return 0;
		}
		if(byteOrder == QSysInfo::LittleEndian)
		{
			return qFromLittleEndian<qint16>(
				ba.constData()
			);
		}
		return qFromBigEndian<qint16>(
			ba.constData()
		);
	}

	qint32 bytesToInt32(
		const QByteArray &ba,
		const QSysInfo::Endian byteOrder
	)
	{
		if(ba.size() != 4)
		{
			return 0;
		};
		if(byteOrder == QSysInfo::LittleEndian)
		{
			return qFromLittleEndian<qint32>(
				ba.constData()
			);
		}
		return qFromBigEndian<qint32>(
			ba.constData()
		);
	}

	qint64 bytesToInt64(
		const QByteArray &ba,
		const QSysInfo::Endian byteOrder
	)
	{
		if(ba.size() != 8)
		{
			return 0;
		}
		if(byteOrder == QSysInfo::LittleEndian)
		{
			return qFromLittleEndian<qint64>(
				ba.constData()
			);
		}
		return qFromBigEndian<qint64>(
			ba.constData()
		);
	}

	quint16 bytesToUInt16(
		const QByteArray &ba,
		const QSysInfo::Endian byteOrder
	)
	{
		return static_cast<quint16>(bytesToInt16(
			ba,
			byteOrder
		));
	}

	quint32 bytesToUInt32(
		const QByteArray &ba,
		const QSysInfo::Endian byteOrder
	)
	{
		return static_cast<quint32>(bytesToInt32(
			ba,
			byteOrder
		));
	}

	quint64 bytesToUInt64(
		const QByteArray &ba,
		const QSysInfo::Endian byteOrder
	)
	{
		return static_cast<quint64>(bytesToInt64(
			ba,
			byteOrder
		));
	}

	qint16 readInt16(
		QIODevice* device,
		const QSysInfo::Endian byteOrder,
		bool* ok
	)
	{
		const QByteArray ba_ = device->read(
			2
		);
		if(ba_.size() != 2)
		{
			*ok = false;
			return 0;
		}
		*ok = true;
		return bytesToInt16(
			ba_,
			byteOrder
		);
	}

	qint32 readInt32(
		QIODevice* device,
		const QSysInfo::Endian byteOrder,
		bool* ok
	)
	{
		const QByteArray ba_ = device->read(
			4
		);
		if(ba_.size() != 4)
		{
			*ok = false;
			return 0;
		}
		*ok = true;
		return bytesToInt32(
			ba_,
			byteOrder
		);
	}

	qint64 readInt64(
		QIODevice* device,
		const QSysInfo::Endian byteOrder,
		bool* ok
	)
	{
		const QByteArray ba_ = device->read(
			8
		);
		if(ba_.size() != 8)
		{
			*ok = false;
			return 0;
		}
		*ok = true;
		return bytesToInt64(
			ba_,
			byteOrder
		);
	}

	quint16 readUInt16(
		QIODevice* device,
		const QSysInfo::Endian byteOrder,
		bool* ok
	)
	{
		return static_cast<quint16>(readInt16(
			device,
			byteOrder,
			ok
		));
	}

	quint32 readUInt32(
		QIODevice* device,
		const QSysInfo::Endian byteOrder,
		bool* ok
	)
	{
		return static_cast<quint32>(readInt32(
			device,
			byteOrder,
			ok
		));
	}

	quint64 readUInt64(
		QIODevice* device,
		const QSysInfo::Endian byteOrder,
		bool* ok
	)
	{
		return static_cast<quint64>(readInt64(
			device,
			byteOrder,
			ok
		));
	}

	QByteArray int16ToBytes(
		const qint16 num,
		const QSysInfo::Endian byteOrder
	)
	{
		QByteArray ba_;
		ba_.resize(
			2
		);
		if(byteOrder == QSysInfo::LittleEndian)
		{
			qToLittleEndian<qint16>(
				num,
				ba_.data()
			);
		}
		else
		{
			qToBigEndian<qint64>(
				num,
				ba_.data()
			);
		}
		return ba_;
	}

	QByteArray int32ToBytes(
		const qint32 num,
		const QSysInfo::Endian byteOrder
	)
	{
		QByteArray ba_;
		ba_.resize(
			4
		);
		if(byteOrder == QSysInfo::LittleEndian)
		{
			qToLittleEndian<qint32>(
				num,
				ba_.data()
			);
		}
		else
		{
			qToBigEndian<qint32>(
				num,
				ba_.data()
			);
		}
		return ba_;
	}

	QByteArray int64ToBytes(
		const qint64 num,
		const QSysInfo::Endian byteOrder
	)
	{
		QByteArray ba_;
		ba_.resize(
			8
		);
		if(byteOrder == QSysInfo::LittleEndian)
		{
			qToLittleEndian<qint64>(
				num,
				ba_.data()
			);
		}
		else
		{
			qToBigEndian<qint64>(
				num,
				ba_.data()
			);
		}
		return ba_;
	}

	bool writeInt16(
		const qint16 num,
		QIODevice* device,
		const QSysInfo::Endian byteOrder
	)
	{
		const QByteArray ba_ = int16ToBytes(
			num,
			byteOrder
		);
		const qint64 bytesWritten_ = device->write(
			ba_
		);
		return (bytesWritten_ == ba_.size());
	}

	bool writeInt32(
		const qint32 num,
		QIODevice* device,
		const QSysInfo::Endian byteOrder
	)
	{
		const QByteArray ba_ = int32ToBytes(
			num,
			byteOrder
		);
		const qint64 bytesWritten_ = device->write(
			ba_
		);
		return (bytesWritten_ == ba_.size());
	}

	bool writeInt64(
		const qint64 num,
		QIODevice* device,
		const QSysInfo::Endian byteOrder
	)
	{
		const QByteArray ba_ = int64ToBytes(
			num,
			byteOrder
		);
		const qint64 bytesWritten_ = device->write(
			ba_
		);
		return (bytesWritten_ == ba_.size());
	}
} // namespace Endian
