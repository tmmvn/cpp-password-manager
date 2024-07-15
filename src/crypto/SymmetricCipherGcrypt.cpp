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
#include "SymmetricCipherGcrypt.h"
#include "config-keepassx.h"
#include "crypto/Crypto.h"

SymmetricCipherGcrypt::SymmetricCipherGcrypt(
	const SymmetricCipher::Algorithm algo,
	const SymmetricCipher::Mode mode,
	const SymmetricCipher::Direction direction
)
	: ctx(
		nullptr
	),
	algo(
		gcryptAlgo(
			algo
		)
	),
	mode(
		gcryptMode(
			mode
		)
	),
	direction(
		direction
	),
	blockSize(
		-1
	)
{
}

SymmetricCipherGcrypt::~SymmetricCipherGcrypt()
{
	gcry_cipher_close(
		ctx
	);
}

int SymmetricCipherGcrypt::gcryptAlgo(
	const SymmetricCipher::Algorithm algo
)
{
	switch(algo)
	{
		case SymmetricCipher::Aes256:
			return GCRY_CIPHER_AES256;
		case SymmetricCipher::Twofish:
			return GCRY_CIPHER_TWOFISH;
		case SymmetricCipher::Salsa20:
			return GCRY_CIPHER_SALSA20;
		default:
			return -1;
	}
}

int SymmetricCipherGcrypt::gcryptMode(
	const SymmetricCipher::Mode mode
)
{
	switch(mode)
	{
		case SymmetricCipher::Ecb:
			return GCRY_CIPHER_MODE_ECB;
		case SymmetricCipher::Cbc:
			return GCRY_CIPHER_MODE_CBC;
		case SymmetricCipher::Stream:
			return GCRY_CIPHER_MODE_STREAM;
		default:
			return -1;
	}
}

void SymmetricCipherGcrypt::setErrorString(
	const gcry_error_t err
)
{
	const char* gcryptError_ = gcry_strerror(
		err
	);
	const char* gcryptErrorSource_ = gcry_strsource(
		err
	);
	this->errorString = QString(
		"%1/%2"
	).arg(
		QString::fromLocal8Bit(
			gcryptErrorSource_
		),
		QString::fromLocal8Bit(
			gcryptError_
		)
	);
}

bool SymmetricCipherGcrypt::init()
{
	if(!Crypto::getInitalized())
	{
		return false;
	}
	gcry_error_t error_ = gcry_cipher_open(
		&this->ctx,
		this->algo,
		this->mode,
		0
	);
	if(error_ != 0)
	{
		this->setErrorString(
			error_
		);
		return false;
	}
	size_t blockSizeT_;
	error_ = gcry_cipher_algo_info(
		this->algo,
		GCRYCTL_GET_BLKLEN,
		nullptr,
		&blockSizeT_
	);
	if(error_ != 0)
	{
		this->setErrorString(
			error_
		);
		return false;
	}
	this->blockSize = static_cast<qint64>(blockSizeT_);
	return true;
}

bool SymmetricCipherGcrypt::setKey(
	const QByteArray &key
)
{
	this->key = key;
	if(const gcry_error_t error_ = gcry_cipher_setkey(
			this->ctx,
			key.constData(),
			key.size()
		);
		error_ != 0)
	{
		this->setErrorString(
			error_
		);
		return false;
	}
	return true;
}

bool SymmetricCipherGcrypt::setIv(
	const QByteArray &iv
)
{
	this->iv = iv;
	if(const gcry_error_t error_ = gcry_cipher_setiv(
			this->ctx,
			iv.constData(),
			iv.size()
		);
		error_ != 0)
	{
		this->setErrorString(
			error_
		);
		return false;
	}
	return true;
}

QByteArray SymmetricCipherGcrypt::process(
	const QByteArray &data,
	bool* ok
)
{
	// TODO: check block size
	QByteArray result_;
	result_.resize(
		data.size()
	);
	gcry_error_t error_;
	if(this->direction == SymmetricCipher::Decrypt)
	{
		error_ = gcry_cipher_decrypt(
			this->ctx,
			result_.data(),
			data.size(),
			data.constData(),
			data.size()
		);
	}
	else
	{
		error_ = gcry_cipher_encrypt(
			this->ctx,
			result_.data(),
			data.size(),
			data.constData(),
			data.size()
		);
	}
	if(error_ != 0)
	{
		this->setErrorString(
			error_
		);
		*ok = false;
		return QByteArray();
	}
	*ok = true;
	return result_;
}

bool SymmetricCipherGcrypt::processInPlace(
	QByteArray &data
)
{
	// TODO: check block size
	gcry_error_t error_;
	if(this->direction == SymmetricCipher::Decrypt)
	{
		error_ = gcry_cipher_decrypt(
			this->ctx,
			data.data(),
			data.size(),
			nullptr,
			0
		);
	}
	else
	{
		error_ = gcry_cipher_encrypt(
			this->ctx,
			data.data(),
			data.size(),
			nullptr,
			0
		);
	}
	if(error_ != 0)
	{
		this->setErrorString(
			error_
		);
		return false;
	}
	return true;
}

bool SymmetricCipherGcrypt::processInPlace(
	QByteArray &data,
	const quint64 rounds
)
{
	// TODO: check block size
	gcry_error_t error_;
	char* rawData_ = data.data();
	const qint64 size_ = data.size();
	if(this->direction == SymmetricCipher::Decrypt)
	{
		for(quint64 i_ = 0; i_ != rounds; ++i_)
		{
			error_ = gcry_cipher_decrypt(
				this->ctx,
				rawData_,
				size_,
				nullptr,
				0
			);
			if(error_ != 0)
			{
				this->setErrorString(
					error_
				);
				return false;
			}
		}
	}
	else
	{
		for(quint64 i_ = 0; i_ != rounds; ++i_)
		{
			error_ = gcry_cipher_encrypt(
				this->ctx,
				rawData_,
				size_,
				nullptr,
				0
			);
			if(error_ != 0)
			{
				this->setErrorString(
					error_
				);
				return false;
			}
		}
	}
	return true;
}

bool SymmetricCipherGcrypt::reset()
{
	gcry_error_t error_ = gcry_cipher_reset(
		this->ctx
	);
	if(error_ != 0)
	{
		this->setErrorString(
			error_
		);
		return false;
	}
	error_ = gcry_cipher_setiv(
		this->ctx,
		this->iv.constData(),
		this->iv.size()
	);
	if(error_ != 0)
	{
		this->setErrorString(
			error_
		);
		return false;
	}
	return true;
}

qint64 SymmetricCipherGcrypt::getBlockSize() const
{
	return this->blockSize;
}

QString SymmetricCipherGcrypt::getErrorString() const
{
	return this->errorString;
}
