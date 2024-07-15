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
#include "HashedBlockStream.h"
#include <cstring>
#include "core/Endian.h"
#include "crypto/CryptoHash.h"
const QSysInfo::Endian HashedBlockStream::ByteOrder = QSysInfo::LittleEndian;

HashedBlockStream::HashedBlockStream(
	QIODevice* baseDevice
)
	: LayeredStream(
		baseDevice
	),
	blockSize(
		1024 * 1024
	),
	bufferPos(),
	blockIndex(),
	eof(),
	error()
{
	this->init();
}

HashedBlockStream::HashedBlockStream(
	QIODevice* baseDevice,
	const qint32 blockSize
)
	: LayeredStream(
		baseDevice
	),
	blockSize(
		blockSize
	),
	bufferPos(),
	blockIndex(),
	eof(),
	error()
{
	this->init();
}

HashedBlockStream::~HashedBlockStream()
{
	this->close();
}

void HashedBlockStream::init()
{
	this->buffer.clear();
	this->bufferPos = 0;
	this->blockIndex = 0;
	this->eof = false;
	this->error = false;
}

bool HashedBlockStream::reset()
{
	// Write final block(s) only if device is writable and we haven't
	// already written a final block.
	if(this->isWritable() && (!this->buffer.isEmpty() || this->blockIndex != 0))
	{
		if(!this->buffer.isEmpty())
		{
			if(!this->writeHashedBlock())
			{
				return false;
			}
		}
		// write empty final block
		if(!this->writeHashedBlock())
		{
			return false;
		}
	}
	this->init();
	return true;
}

void HashedBlockStream::close()
{
	// Write final block(s) only if device is writable and we haven't
	// already written a final block.
	if(this->isWritable() && (!this->buffer.isEmpty() || this->blockIndex != 0))
	{
		if(!this->buffer.isEmpty())
		{
			this->writeHashedBlock();
		}
		// write empty final block
		this->writeHashedBlock();
	}
	LayeredStream::close();
}

qint64 HashedBlockStream::readData(
	char* data,
	const qint64 maxSize
)
{
	if(this->error)
	{
		return -1;
	}
	if(this->eof)
	{
		return 0;
	}
	auto bytesRemaining_ = static_cast<qint32>(maxSize);
	auto offset_ = 0;
	while(bytesRemaining_ > 0)
	{
		if(this->bufferPos == this->buffer.size())
		{
			if(!this->readHashedBlock())
			{
				if(this->error)
				{
					return -1;
				}
				return maxSize - bytesRemaining_;
			}
		}
		const qint32 bytesToCopy_ = qMin(
			bytesRemaining_,
			static_cast<qint32>(this->buffer.size()) - this->bufferPos
		);
		memcpy(
			data + offset_,
			this->buffer.constData() + this->bufferPos,
			bytesToCopy_
		);
		offset_ += bytesToCopy_;
		this->bufferPos += bytesToCopy_;
		bytesRemaining_ -= bytesToCopy_;
	}
	return maxSize;
}

bool HashedBlockStream::readHashedBlock()
{
	bool ok_;
	if(const qint32 index_ = Endian::readInt32(
			this->getBaseDevice(),
			this->ByteOrder,
			&ok_
		);
		!ok_ || index_ != this->blockIndex)
	{
		this->error = true;
		this->setErrorString(
			"Invalid block index."
		);
		return false;
	}
	const QByteArray hash_ = this->getBaseDevice()->read(
		32
	);
	if(hash_.size() != 32)
	{
		this->error = true;
		this->setErrorString(
			"Invalid hash size."
		);
		return false;
	}
	this->blockSize = Endian::readInt32(
		this->getBaseDevice(),
		this->ByteOrder,
		&ok_
	);
	if(!ok_ || this->blockSize < 0)
	{
		this->error = true;
		this->setErrorString(
			"Invalid block size."
		);
		return false;
	}
	if(this->blockSize == 0)
	{
		if(hash_.count(
			'\0'
		) != 32)
		{
			this->error = true;
			this->setErrorString(
				"Invalid hash of final block."
			);
			return false;
		}
		this->eof = true;
		return false;
	}
	this->buffer = this->getBaseDevice()->read(
		this->blockSize
	);
	if(this->buffer.size() != this->blockSize)
	{
		this->error = true;
		this->setErrorString(
			"Block too short."
		);
		return false;
	}
	if(hash_ != CryptoHash::hash(
		this->buffer,
		CryptoHash::Sha256
	))
	{
		this->error = true;
		this->setErrorString(
			"Mismatch between hash and data."
		);
		return false;
	}
	this->bufferPos = 0;
	this->blockIndex++;
	return true;
}

qint64 HashedBlockStream::writeData(
	const char* data,
	const qint64 maxSize
)
{
	if(maxSize <= 0)
	{
		return 0;
	}
	if(this->error)
	{
		return -1;
	}
	auto bytesRemaining_ = static_cast<qint32>(maxSize);
	auto offset_ = 0;
	while(bytesRemaining_ > 0)
	{
		const qint32 bytesToCopy_ = qMin(
			bytesRemaining_,
			this->blockSize - static_cast<qint32>(this->buffer.size())
		);
		this->buffer.append(
			data + offset_,
			bytesToCopy_
		);
		offset_ += bytesToCopy_;
		bytesRemaining_ -= bytesToCopy_;
		if(this->buffer.size() == this->blockSize)
		{
			if(!this->writeHashedBlock())
			{
				if(this->error)
				{
					return -1;
				}
				return maxSize - bytesRemaining_;
			}
		}
	}
	return maxSize;
}

bool HashedBlockStream::writeHashedBlock()
{
	if(!Endian::writeInt32(
		static_cast<qint32>(this->blockIndex),
		this->getBaseDevice(),
		this->ByteOrder
	))
	{
		this->error = true;
		this->setErrorString(
			this->getBaseDevice()->errorString()
		);
		return false;
	}
	this->blockIndex++;
	QByteArray hash_;
	if(!this->buffer.isEmpty())
	{
		hash_ = CryptoHash::hash(
			this->buffer,
			CryptoHash::Sha256
		);
	}
	else
	{
		hash_.fill(
			0,
			32
		);
	}
	if(this->getBaseDevice()->write(
		hash_
	) != hash_.size())
	{
		this->error = true;
		this->setErrorString(
			this->getBaseDevice()->errorString()
		);
		return false;
	}
	if(!Endian::writeInt32(
		static_cast<qint32>(this->buffer.size()),
		this->getBaseDevice(),
		this->ByteOrder
	))
	{
		this->error = true;
		this->setErrorString(
			this->getBaseDevice()->errorString()
		);
		return false;
	}
	if(!this->buffer.isEmpty())
	{
		if(this->getBaseDevice()->write(
			this->buffer
		) != this->buffer.size())
		{
			this->error = true;
			this->setErrorString(
				this->getBaseDevice()->errorString()
			);
			return false;
		}
		this->buffer.clear();
	}
	return true;
}
