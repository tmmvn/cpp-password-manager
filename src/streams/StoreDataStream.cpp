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
#include "StoreDataStream.h"

StoreDataStream::StoreDataStream(
	QIODevice* baseDevice
)
	: LayeredStream(
		baseDevice
	)
{
}

bool StoreDataStream::open(
	const OpenMode mode
)
{
	const bool result_ = LayeredStream::open(
		mode
	);
	if(result_)
	{
		this->storedData.clear();
	}
	return result_;
}

qint64 StoreDataStream::readData(
	char* data,
	const qint64 maxSize
)
{
	if(maxSize <= 0)
	{
		return 0;
	}
	const qint64 bytesRead_ = LayeredStream::readData(
		data,
		maxSize
	);
	if(bytesRead_ == -1)
	{
		this->setErrorString(
			this->getBaseDevice()->errorString()
		);
		return -1;
	}
	this->storedData.append(
		data,
		bytesRead_
	);
	return bytesRead_;
}
