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
#include "SymmetricCipherStream.h"

SymmetricCipherStream::SymmetricCipherStream(
	QIODevice* baseDevice,
	const SymmetricCipher::Algorithm algo,
	const SymmetricCipher::Mode mode,
	const SymmetricCipher::Direction direction
)
	: LayeredStream(
		baseDevice
	),
	cipher(
		new SymmetricCipher(
			algo,
			mode,
			direction
		)
	),
	bufferPos(
		0
	),
	bufferFilling(
		false
	),
	error(
		false
	),
	isInitalized(
		false
	),
	dataWritten(
		false
	)
{
}

SymmetricCipherStream::~SymmetricCipherStream()
{
	this->close();
}

bool SymmetricCipherStream::init(
	const QByteArray &key,
	const QByteArray &iv
)
{
	this->isInitalized = this->cipher->init(
		key,
		iv
	);
	if(!this->isInitalized)
	{
		this->setErrorString(
			this->cipher->getErrorString()
		);
	}
	return this->isInitalized;
}

void SymmetricCipherStream::resetInternalState()
{
	this->buffer.clear();
	this->bufferPos = 0;
	this->bufferFilling = false;
	this->error = false;
	this->dataWritten = false;
	if(const auto resetSuccesfully_ = this->cipher->reset();
		!resetSuccesfully_)
	{
		this->setErrorString(
			"Failed to reset cypher."
		);
	}
}

bool SymmetricCipherStream::open(
	const OpenMode mode
)
{
	if(!this->isInitalized)
	{
		return false;
	}
	return LayeredStream::open(
		mode
	);
}

bool SymmetricCipherStream::reset()
{
	if(this->isWritable() && this->dataWritten)
	{
		if(!this->writeBlock(
			true
		))
		{
			return false;
		}
	}
	this->resetInternalState();
	return true;
}

void SymmetricCipherStream::close()
{
	if(this->isWritable() && this->dataWritten)
	{
		this->writeBlock(
			true
		);
	}
	this->resetInternalState();
	LayeredStream::close();
}

qint64 SymmetricCipherStream::readData(
	char* data,
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
	qint64 bytesRemaining_ = maxSize;
	qint64 offset_ = 0;
	while(bytesRemaining_ > 0)
	{
		if((this->bufferPos == this->buffer.size()) || this->bufferFilling)
		{
			if(!this->readBlock())
			{
				if(this->error)
				{
					return -1;
				}
				return maxSize - bytesRemaining_;
			}
		}
		const int bytesToCopy_ = qMin(
			static_cast<int>(bytesRemaining_),
			static_cast<int>(this->buffer.size() - this->bufferPos)
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

bool SymmetricCipherStream::readBlock()
{
	QByteArray newData_;
	if(this->bufferFilling)
	{
		newData_.resize(
			this->cipher->getBlockSize() - this->buffer.size()
		);
	}
	else
	{
		this->buffer.clear();
		newData_.resize(
			this->cipher->getBlockSize()
		);
	}
	const auto readResult_ = static_cast<int>(this->getBaseDevice()->read(
		newData_.data(),
		newData_.size()
	));
	if(readResult_ == -1)
	{
		this->error = true;
		this->setErrorString(
			this->getBaseDevice()->errorString()
		);
		return false;
	}
	this->buffer.append(
		newData_.left(
			readResult_
		)
	);
	if(this->buffer.size() != this->cipher->getBlockSize())
	{
		this->bufferFilling = true;
		return false;
	}
	if(!this->cipher->processInPlace(
		this->buffer
	))
	{
		this->error = true;
		this->setErrorString(
			this->cipher->getErrorString()
		);
		return false;
	}
	this->bufferPos = 0;
	this->bufferFilling = false;
	if(this->getBaseDevice()->atEnd())
	{
		const quint8 padLength_ = this->buffer.at(
			this->buffer.size() - 1
		);
		// PKCS7 padding
		if(padLength_ == this->cipher->getBlockSize())
		{
			if(this->buffer != QByteArray(
				this->cipher->getBlockSize(),
				static_cast<char>(this->cipher->getBlockSize())
			))
			{
				this->error = true;
				this->setErrorString(
					"Invalid block size."
				);
				return false; // Or return an error code
			}
			// full block with just padding: discard
			this->buffer.clear();
			return false;
		}
		if(padLength_ > this->cipher->getBlockSize())
		{
			// invalid padding
			this->error = true;
			this->setErrorString(
				"Invalid padding."
			);
			return false;
		}
		if(this->buffer.right(
			padLength_
		) != QByteArray(
			padLength_,
			static_cast<char>(padLength_)
		))
		{
			return false;
		};
		// resize buffer to strip padding
		this->buffer.resize(
			this->cipher->getBlockSize() - padLength_
		);
		return true;
	}
	return true;
}

qint64 SymmetricCipherStream::writeData(
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
	this->dataWritten = true;
	qint64 bytesRemaining_ = maxSize;
	qint64 offset_ = 0;
	while(bytesRemaining_ > 0)
	{
		const int bytesToCopy_ = qMin(
			static_cast<int>(bytesRemaining_),
			static_cast<int>(this->cipher->getBlockSize() - this->buffer.size())
		);
		this->buffer.append(
			data + offset_,
			bytesToCopy_
		);
		offset_ += bytesToCopy_;
		bytesRemaining_ -= bytesToCopy_;
		if(this->buffer.size() == this->cipher->getBlockSize())
		{
			if(!writeBlock(
				false
			))
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

bool SymmetricCipherStream::writeBlock(
	const bool lastBlock
)
{
	if(lastBlock)
	{
		// PKCS7 padding
		const auto padLen_ = static_cast<int>(this->cipher->getBlockSize() -
			this->buffer.size());
		for(auto i_ = 0; i_ < padLen_; i_++)
		{
			this->buffer.append(
				static_cast<char>(padLen_)
			);
		}
	}
	if(!this->cipher->processInPlace(
		this->buffer
	))
	{
		this->error = true;
		this->setErrorString(
			this->cipher->getErrorString()
		);
		return false;
	}
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
	return true;
}
