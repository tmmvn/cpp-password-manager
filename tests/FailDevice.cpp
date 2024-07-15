/*
 *  Copyright (C) 2015 Felix Geyer <debfx@fobos.de>
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
#include "FailDevice.h"

FailDevice::FailDevice(
	const int failAfter,
	QObject* parent
)
	: QBuffer(
		parent
	),
	failAfter(
		failAfter
	),
	readCount(
		0
	),
	writeCount(
		0
	)
{
}

bool FailDevice::open(
	const OpenMode openMode
)
{
	return QBuffer::open(
		openMode | Unbuffered
	);
}

qint64 FailDevice::readData(
	char* data,
	const qint64 len
)
{
	if(this->readCount >= this->failAfter)
	{
		this->setErrorString(
			"FAILDEVICE"
		);
		return -1;
	}
	const qint64 result_ = QBuffer::readData(
		data,
		len
	);
	if(result_ != -1)
	{
		this->readCount += result_;
	}
	return result_;
}

qint64 FailDevice::writeData(
	const char* data,
	const qint64 len
)
{
	if(this->writeCount >= this->failAfter)
	{
		this->setErrorString(
			"FAILDEVICE"
		);
		return -1;
	}
	const qint64 result_ = QBuffer::writeData(
		data,
		len
	);
	if(result_ != -1)
	{
		this->writeCount += result_;
	}
	return result_;
}
