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
#include "KeePass2RandomStream.h"
#include "crypto/CryptoHash.h"
#include "format/KeePass2.h"

KeePass2RandomStream::KeePass2RandomStream()
	: cipher(
		SymmetricCipher::Salsa20,
		SymmetricCipher::Stream,
		SymmetricCipher::Encrypt
	),
	offset(
		0
	)
{
}

bool KeePass2RandomStream::init(
	const QByteArray &key
)
{
	return this->cipher.init(
		CryptoHash::hash(
			key,
			CryptoHash::Sha256
		),
		KeePass2::INNER_STREAM_SALSA20_IV
	);
}

QByteArray KeePass2RandomStream::getRandomBytes(
	const int size,
	bool* ok
)
{
	QByteArray result_;
	int bytesRemaining_ = size;
	while(bytesRemaining_ > 0)
	{
		if(this->buffer.size() == this->offset)
		{
			if(!this->loadBlock())
			{
				*ok = false;
				return QByteArray();
			}
		}
		const int bytesToCopy_ = qMin(
			bytesRemaining_,
			static_cast<int>(buffer.size()) - offset
		);
		result_.append(
			this->buffer.mid(
				this->offset,
				bytesToCopy_
			)
		);
		this->offset += bytesToCopy_;
		bytesRemaining_ -= bytesToCopy_;
	}
	*ok = true;
	return result_;
}

QByteArray KeePass2RandomStream::process(
	const QByteArray &data,
	bool* ok
)
{
	bool randomBytesOk_;
	QByteArray randomData_ = this->getRandomBytes(
		static_cast<int>(data.size()),
		&randomBytesOk_
	);
	if(!randomBytesOk_)
	{
		*ok = false;
		return QByteArray();
	}
	QByteArray result_;
	result_.resize(
		data.size()
	);
	for(auto i_ = 0; i_ < data.size(); i_++)
	{
		result_[i_] = static_cast<char>(data[i_] ^ randomData_[i_]);
	}
	*ok = true;
	return result_;
}

bool KeePass2RandomStream::processInPlace(
	QByteArray &data
)
{
	bool ok_;
	QByteArray randomData_ = this->getRandomBytes(
		static_cast<int>(data.size()),
		&ok_
	);
	if(!ok_)
	{
		return false;
	}
	for(auto i = 0; i < data.size(); i++)
	{
		data[i] = static_cast<char>(data[i] ^ randomData_[i]);
	}
	return true;
}

QString KeePass2RandomStream::getErrorString() const
{
	return this->cipher.getErrorString();
}

bool KeePass2RandomStream::loadBlock()
{
	if(this->offset != this->buffer.size())
	{
		return false;
	}
	this->buffer.fill(
		'\0',
		this->cipher.getBlockSize()
	);
	if(!this->cipher.processInPlace(
		this->buffer
	))
	{
		return false;
	}
	this->offset = 0;
	return true;
}
