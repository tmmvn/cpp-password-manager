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
#include "LayeredStream.h"

LayeredStream::LayeredStream(
	QIODevice* baseDevice
)
	: QIODevice(
		baseDevice
	),
	baseDevice(
		baseDevice
	)
{
	this->connect(
		baseDevice,
		&QIODevice::aboutToClose,
		this,
		&LayeredStream::do_CloseStream
	);
}

LayeredStream::~LayeredStream()
{
	QIODevice::close();
}

bool LayeredStream::isSequential() const
{
	return true;
}

bool LayeredStream::open(
	OpenMode mode
)
{
	if(this->isOpen())
	{
		qWarning(
			"LayeredStream::open: Device is already open."
		);
		return false;
	}
	const bool readMode_ = (mode & ReadOnly);
	const bool writeMode_ = (mode & WriteOnly);
	if(readMode_ && writeMode_)
	{
		qWarning(
			"LayeredStream::open: Reading and writing at the same time is not supported."
		);
		return false;
	}
	if(!readMode_ && !writeMode_)
	{
		qWarning(
			"LayeredStream::open: Must be opened in read or write mode."
		);
		return false;
	}
	if((readMode_ && !baseDevice->isReadable()) || (writeMode_ && !baseDevice->
		isWritable()))
	{
		qWarning(
			"LayeredStream::open: Base device is not opened correctly."
		);
		return false;
	}
	if(mode & Append)
	{
		qWarning(
			"LayeredStream::open: QIODevice::Append is not supported."
		);
		mode = mode & ~Append;
	}
	if(mode & Truncate)
	{
		qWarning(
			"LayeredStream::open: QIODevice::Truncate is not supported."
		);
		mode = mode & ~Truncate;
	}
	mode = mode | Unbuffered;
	return QIODevice::open(
		mode
	);
}

qint64 LayeredStream::readData(
	char* data,
	const qint64 maxSize
)
{
	return baseDevice->read(
		data,
		maxSize
	);
}

qint64 LayeredStream::writeData(
	const char* data,
	const qint64 maxSize
)
{
	return baseDevice->write(
		data,
		maxSize
	);
}

void LayeredStream::do_CloseStream()
{
	this->close();
}
