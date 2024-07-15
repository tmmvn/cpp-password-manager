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
#include "KeePass2Writer.h"
#include <QBuffer>
#include <QFile>
#include <QIODevice>
#include "core/Database.h"
#include "core/Endian.h"
#include "crypto/CryptoHash.h"
#include "crypto/Random.h"
#include "format/KeePass2RandomStream.h"
#include "format/KeePass2XmlWriter.h"
#include "streams/HashedBlockStream.h"
#include "streams/QtIOCompressor"
#include "streams/SymmetricCipherStream.h"
#define CHECK_RETURN(x) if (!(x)) return;
#define CHECK_RETURN_FALSE(x) if (!(x)) return false;

KeePass2Writer::KeePass2Writer()
	: device(
		nullptr
	),
	error(
		false
	)
{
}

void KeePass2Writer::writeDatabase(
	QIODevice* device,
	Database* db
)
{
	if(device == nullptr)
	{
		raiseError(
			"Null device"
		);
		return;
	}
	if(db == nullptr)
	{
		raiseError(
			"Null database"
		);
		return;
	}
	this->error = false;
	this->errorStr.clear();
	QByteArray masterSeed_ = Random::getInstance()->getRandomArray(
		32
	);
	QByteArray encryptionIV_ = Random::getInstance()->getRandomArray(
		16
	);
	QByteArray protectedStreamKey_ = Random::getInstance()->getRandomArray(
		32
	);
	QByteArray startBytes_ = Random::getInstance()->getRandomArray(
		32
	);
	QByteArray endOfHeader_ = "\r\n\r\n";
	CryptoHash hash_(
		CryptoHash::Sha256
	);
	hash_.addData(
		masterSeed_
	);
	if(db->transformedMasterKey().isEmpty())
	{
		this->raiseError(
			"No transformed master key"
		);
		return;
	}
	hash_.addData(
		db->transformedMasterKey()
	);
	QByteArray finalKey_ = hash_.getResult();
	QBuffer header_;
	header_.open(
		QIODevice::WriteOnly
	);
	this->device = &header_;
	CHECK_RETURN(
		this->writeData(Endian::int32ToBytes(KeePass2::SIGNATURE_1, KeePass2::
			BYTEORDER))
	);
	CHECK_RETURN(
		this->writeData(Endian::int32ToBytes(KeePass2::SIGNATURE_2, KeePass2::
			BYTEORDER))
	);
	CHECK_RETURN(
		this->writeData(Endian::int32ToBytes(KeePass2::FILE_VERSION, KeePass2::
			BYTEORDER))
	);
	CHECK_RETURN(
		this->writeHeaderField(KeePass2::CipherID, db->getCipher().toByteArray()
		)
	);
	CHECK_RETURN(
		this->writeHeaderField(KeePass2::CompressionFlags, Endian::int32ToBytes(
			db-> getCompressionAlgo(), KeePass2::BYTEORDER))
	);
	CHECK_RETURN(
		this->writeHeaderField(KeePass2::MasterSeed, masterSeed_)
	);
	CHECK_RETURN(
		this->writeHeaderField(KeePass2::TransformSeed, db->transformSeed())
	);
	CHECK_RETURN(
		this->writeHeaderField(KeePass2::TransformRounds, Endian::int64ToBytes(
			db-> transformRounds(), KeePass2::BYTEORDER))
	);
	CHECK_RETURN(
		this->writeHeaderField(KeePass2::EncryptionIV, encryptionIV_)
	);
	CHECK_RETURN(
		this->writeHeaderField(KeePass2::ProtectedStreamKey, protectedStreamKey_
		)
	);
	CHECK_RETURN(
		this->writeHeaderField(KeePass2::StreamStartBytes, startBytes_)
	);
	CHECK_RETURN(
		this->writeHeaderField(KeePass2::InnerRandomStreamID, Endian::
			int32ToBytes( KeePass2::Salsa20, KeePass2::BYTEORDER))
	);
	CHECK_RETURN(
		this->writeHeaderField(KeePass2::EndOfHeader, endOfHeader_)
	);
	header_.close();
	this->device = device;
	QByteArray headerHash_ = CryptoHash::hash(
		header_.data(),
		CryptoHash::Sha256
	);
	CHECK_RETURN(
		this->writeData(header_.data())
	);
	SymmetricCipherStream cipherStream_(
		device,
		SymmetricCipher::Aes256,
		SymmetricCipher::Cbc,
		SymmetricCipher::Encrypt
	);
	cipherStream_.init(
		finalKey_,
		encryptionIV_
	);
	if(!cipherStream_.open(
		QIODevice::WriteOnly
	))
	{
		this->raiseError(
			cipherStream_.errorString()
		);
		return;
	}
	this->device = &cipherStream_;
	CHECK_RETURN(
		this->writeData(startBytes_)
	);
	HashedBlockStream hashedStream_(
		&cipherStream_
	);
	if(!hashedStream_.open(
		QIODevice::WriteOnly
	))
	{
		this->raiseError(
			hashedStream_.errorString()
		);
		return;
	}
	std::unique_ptr<QtIOCompressor> ioCompressor_;
	if(db->getCompressionAlgo() == Database::CompressionNone)
	{
		this->device = &hashedStream_;
	}
	else
	{
		ioCompressor_.reset(
			new QtIOCompressor(
				&hashedStream_
			)
		);
		ioCompressor_->setStreamFormat(
			QtIOCompressor::GzipFormat
		);
		if(!ioCompressor_->open(
			QIODevice::WriteOnly
		))
		{
			this->raiseError(
				ioCompressor_->errorString()
			);
			return;
		}
		this->device = ioCompressor_.get();
	}
	KeePass2RandomStream randomStream_;
	if(!randomStream_.init(
		protectedStreamKey_
	))
	{
		this->raiseError(
			randomStream_.getErrorString()
		);
		return;
	}
	KeePass2XmlWriter xmlWriter_;
	xmlWriter_.writeDatabase(
		this->device,
		db,
		&randomStream_,
		headerHash_
	);
	// Explicitly close/reset streams so they are flushed and we can detect
	// errors. QIODevice::close() resets errorString() etc.
	if(ioCompressor_)
	{
		ioCompressor_->close();
	}
	if(!hashedStream_.reset())
	{
		this->raiseError(
			hashedStream_.errorString()
		);
		return;
	}
	if(!cipherStream_.reset())
	{
		this->raiseError(
			cipherStream_.errorString()
		);
		return;
	}
	if(xmlWriter_.hasError())
	{
		this->raiseError(
			xmlWriter_.getErrorString()
		);
	}
}

bool KeePass2Writer::writeData(
	const QByteArray &data
)
{
	if(this->device->write(
		data
	) != data.size())
	{
		this->raiseError(
			this->device->errorString()
		);
		return false;
	}
	return true;
}

bool KeePass2Writer::writeHeaderField(
	const KeePass2::HeaderFieldID fieldId,
	const QByteArray &data
)
{
	if(data.size() > 65535)
	{
		this->raiseError(
			"Header field too large"
		);
		return false;
	}
	QByteArray fieldIdArr_;
	fieldIdArr_.resize(
		1
	);
	fieldIdArr_[0] = fieldId;
	CHECK_RETURN_FALSE(
		this->writeData(fieldIdArr_)
	);
	CHECK_RETURN_FALSE(
		this->writeData(Endian::int16ToBytes(static_cast<quint16>(data.size()),
			KeePass2::BYTEORDER))
	);
	CHECK_RETURN_FALSE(
		this->writeData(data)
	);
	return true;
}

void KeePass2Writer::writeDatabase(
	const QString &filename,
	Database* db
)
{
	QFile file_(
		filename
	);
	if(!file_.open(
		QIODevice::WriteOnly | QIODevice::Truncate
	))
	{
		this->raiseError(
			file_.errorString()
		);
		return;
	}
	this->writeDatabase(
		&file_,
		db
	);
}

bool KeePass2Writer::hasError() const
{
	return this->error;
}

QString KeePass2Writer::getErrorString()
{
	return this->errorStr;
}

void KeePass2Writer::raiseError(
	const QString &errorMessage
)
{
	qCritical() << errorMessage;
	this->error = true;
	this->errorStr = errorMessage;
}
