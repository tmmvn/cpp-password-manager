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
#include "KeePass2Reader.h"
#include <QBuffer>
#include <QFile>
#include <QIODevice>
#include "core/Database.h"
#include "core/Endian.h"
#include "crypto/CryptoHash.h"
#include "format/KeePass2.h"
#include "format/KeePass2RandomStream.h"
#include "format/KeePass2XmlReader.h"
#include "streams/HashedBlockStream.h"
#include "streams/QtIOCompressor"
#include "streams/StoreDataStream.h"
#include "streams/SymmetricCipherStream.h"

KeePass2Reader::KeePass2Reader()
	: device(
		nullptr
	),
	headerStream(
		nullptr
	),
	error(
		false
	),
	headerEnd(
		false
	),
	saveXml(
		true
	),
	db(
		nullptr
	)
{
}

Database* KeePass2Reader::readDatabase(
	QIODevice* device,
	const CompositeKey &key,
	bool keepDatabase
)
{
	if(device == nullptr)
	{
		this->raiseError(
			"Null device"
		);
		return nullptr;
	}
	this->db = new Database();
	this->device = device;
	this->error = false;
	this->errorStr.clear();
	this->headerEnd = false;
	this->xmlData.clear();
	this->masterSeed.clear();
	this->transformSeed.clear();
	this->encryptionIV.clear();
	this->streamStartBytes.clear();
	this->protectedStreamKey.clear();
	StoreDataStream headerStream_(
		this->device
	);
	headerStream_.open(
		QIODevice::ReadOnly
	);
	this->headerStream = &headerStream_;
	bool ok_;
	if(quint32 signature1_ = Endian::readUInt32(
			this->headerStream,
			KeePass2::BYTEORDER,
			&ok_
		);
		!ok_ || signature1_ != KeePass2::SIGNATURE_1)
	{
		this->raiseError(
			this->tr(
				"Not a KeePass database."
			)
		);
		return nullptr;
	}
	if(quint32 signature2_ = Endian::readUInt32(
			this->headerStream,
			KeePass2::BYTEORDER,
			&ok_
		);
		!ok_ || signature2_ != KeePass2::SIGNATURE_2)
	{
		this->raiseError(
			this->tr(
				"Not a KeePass database."
			)
		);
		return nullptr;
	}
	quint32 version_ = Endian::readUInt32(
		this->headerStream,
		KeePass2::BYTEORDER,
		&ok_
	) & KeePass2::FILE_VERSION_CRITICAL_MASK;
	if(quint32 maxVersion_ = KeePass2::FILE_VERSION &
			KeePass2::FILE_VERSION_CRITICAL_MASK;
		!ok_ || version_ < KeePass2::FILE_VERSION_MIN || version_ > maxVersion_)
	{
		this->raiseError(
			this->tr(
				"Unsupported KeePass database version."
			)
		);
		return nullptr;
	}
	while(this->readHeaderField() && !this->hasError())
	{
	}
	headerStream_.close();
	if(this->hasError())
	{
		this->raiseError(
			this->tr(
				"Error reading header stream"
			)
		);
		return nullptr;
	}
	// check if all required headers were present
	if(this->masterSeed.isEmpty() || this->transformSeed.isEmpty() || this->
		encryptionIV.isEmpty() || this->streamStartBytes.isEmpty() || this->
		protectedStreamKey.isEmpty() || this->db->getCipher().isNull())
	{
		this->raiseError(
			"missing database headers"
		);
		return nullptr;
	}
	if(!this->db->setKey(
		key,
		this->transformSeed,
		false
	))
	{
		this->raiseError(
			this->tr(
				"Unable to calculate master key"
			)
		);
		return nullptr;
	}
	CryptoHash hash_(
		CryptoHash::Sha256
	);
	hash_.addData(
		this->masterSeed
	);
	hash_.addData(
		this->db->transformedMasterKey()
	);
	QByteArray finalKey_ = hash_.getResult();
	SymmetricCipherStream cipherStream_(
		device,
		SymmetricCipher::Aes256,
		SymmetricCipher::Cbc,
		SymmetricCipher::Decrypt
	);
	if(!cipherStream_.init(
		finalKey_,
		this->encryptionIV
	))
	{
		this->raiseError(
			cipherStream_.errorString()
		);
		return nullptr;
	}
	if(!cipherStream_.open(
		QIODevice::ReadOnly
	))
	{
		this->raiseError(
			cipherStream_.errorString()
		);
		return nullptr;
	}
	if(QByteArray realStart_ = cipherStream_.read(
			32
		);
		realStart_ != this->streamStartBytes)
	{
		this->raiseError(
			this->tr(
				"Wrong key or database file is corrupt."
			)
		);
		return nullptr;
	}
	auto hashedStream_ = new HashedBlockStream(
		&cipherStream_
	);
	if(!hashedStream_->open(
		QIODevice::ReadOnly
	))
	{
		this->raiseError(
			hashedStream_->errorString()
		);
		delete hashedStream_;
		return nullptr;
	}
	QIODevice* xmlDevice_ = nullptr;
	if(this->db->getCompressionAlgo() == Database::CompressionNone)
	{
		xmlDevice_ = hashedStream_;
	}
	else
	{
		const auto ioCompressor_ = new QtIOCompressor(
			hashedStream_
		);
		ioCompressor_->setStreamFormat(
			QtIOCompressor::GzipFormat
		);
		if(!ioCompressor_->open(
			QIODevice::ReadOnly
		))
		{
			this->raiseError(
				ioCompressor_->errorString()
			);
			delete ioCompressor_;
			delete hashedStream_;
			return nullptr;
		}
		xmlDevice_ = ioCompressor_;
	}
	KeePass2RandomStream randomStream_;
	if(!randomStream_.init(
		this->protectedStreamKey
	))
	{
		this->raiseError(
			randomStream_.getErrorString()
		);
		delete xmlDevice_;
		return nullptr;
	}
	KeePass2XmlReader xmlReader_;
	if(this->saveXml)
	{
		this->xmlData = xmlDevice_->readAll();
		const auto buffer_ = new QBuffer(
			&this->xmlData
		);
		buffer_->open(
			QIODevice::ReadOnly
		);
		xmlReader_.readDatabase(
			buffer_,
			this->db,
			&randomStream_
		);
		delete buffer_;
	}
	else
	{
		xmlReader_.readDatabase(
			xmlDevice_,
			this->db,
			&randomStream_
		);
	}
	if(xmlReader_.hasError())
	{
		this->raiseError(
			xmlReader_.getErrorString()
		);
		if(!keepDatabase)
		{
			delete this->db;
			this->db = nullptr;
		}
		delete xmlDevice_;
		return this->db;
	}
	if(!xmlReader_.getHeaderHash().isEmpty())
	{
		if(QByteArray headerHash_ = CryptoHash::hash(
				headerStream_.getStoredData(),
				CryptoHash::Sha256
			);
			headerHash_ != xmlReader_.getHeaderHash())
		{
			this->raiseError(
				"Header doesn't match hash"
			);
			if(!keepDatabase)
			{
				delete this->db;
				this->db = nullptr;
			}
		}
	}
	delete xmlDevice_;
	return this->db;
}

Database* KeePass2Reader::readDatabase(
	const QString &filename,
	const CompositeKey &key
)
{
	QFile file_(
		filename
	);
	if(!file_.open(
		QFile::ReadOnly
	))
	{
		this->raiseError(
			file_.errorString()
		);
		return nullptr;
	}
	Database* db_ = this->readDatabase(
		&file_,
		key,
		false
	);
	if(file_.error() != QFile::NoError)
	{
		this->raiseError(
			file_.errorString()
		);
		if(db_ != nullptr)
		{
			delete db_;
			db_ = nullptr;
		}
	}
	return db_;
}

bool KeePass2Reader::hasError() const
{
	return this->error;
}

QString KeePass2Reader::getErrorString()
{
	return this->errorStr;
}

void KeePass2Reader::setSaveXml(
	const bool save
)
{
	this->saveXml = save;
}

QByteArray KeePass2Reader::getXMLData()
{
	return this->xmlData;
}

QByteArray KeePass2Reader::getStreamKey()
{
	return this->protectedStreamKey;
}

void KeePass2Reader::raiseError(
	const QString &errorMessage
)
{
	qCritical() << errorMessage;
	this->error = true;
	this->errorStr = errorMessage;
}

bool KeePass2Reader::readHeaderField()
{
	const QByteArray fieldIDArray_ = this->headerStream->read(
		1
	);
	if(fieldIDArray_.size() != 1)
	{
		this->raiseError(
			"Invalid header id size"
		);
		return false;
	}
	const quint8 fieldID_ = fieldIDArray_.at(
		0
	);
	bool ok_;
	const quint16 fieldLen_ = Endian::readUInt16(
		this->headerStream,
		KeePass2::BYTEORDER,
		&ok_
	);
	if(!ok_)
	{
		this->raiseError(
			"Invalid header field length"
		);
		return false;
	}
	QByteArray fieldData_;
	if(fieldLen_ != 0)
	{
		fieldData_ = this->headerStream->read(
			fieldLen_
		);
		if(fieldData_.size() != fieldLen_)
		{
			this->raiseError(
				"Invalid header data length"
			);
			return false;
		}
	}
	switch(fieldID_)
	{
		case KeePass2::EndOfHeader:
			this->headerEnd = true;
			break;
		case KeePass2::CipherID:
			this->setCipher(
				fieldData_
			);
			break;
		case KeePass2::CompressionFlags:
			this->setCompressionFlags(
				fieldData_
			);
			break;
		case KeePass2::MasterSeed:
			this->setMasterSeed(
				fieldData_
			);
			break;
		case KeePass2::TransformSeed:
			this->setTransformSeed(
				fieldData_
			);
			break;
		case KeePass2::TransformRounds:
			this->setTansformRounds(
				fieldData_
			);
			break;
		case KeePass2::EncryptionIV:
			this->setEncryptionIV(
				fieldData_
			);
			break;
		case KeePass2::ProtectedStreamKey:
			this->setProtectedStreamKey(
				fieldData_
			);
			break;
		case KeePass2::StreamStartBytes:
			this->setStreamStartBytes(
				fieldData_
			);
			break;
		case KeePass2::InnerRandomStreamID:
			this->setInnerRandomStreamID(
				fieldData_
			);
			break;
		default: qWarning(
				"Unknown header field read: id=%d",
				fieldID_
			);
			break;
	}
	return !this->headerEnd;
}

void KeePass2Reader::setCipher(
	const QByteArray &data
)
{
	if(data.size() != UUID::Length)
	{
		this->raiseError(
			"Invalid cipher uuid length"
		);
	}
	else
	{
		if(const UUID uuid_(
				data
			);
			uuid_ != KeePass2::CIPHER_AES)
		{
			this->raiseError(
				"Unsupported cipher"
			);
		}
		else
		{
			this->db->setCipher(
				uuid_
			);
		}
	}
}

void KeePass2Reader::setCompressionFlags(
	const QByteArray &data
)
{
	if(data.size() != 4)
	{
		this->raiseError(
			"Invalid compression flags length"
		);
	}
	else
	{
		if(quint32 id_ = Endian::bytesToUInt32(
				data,
				KeePass2::BYTEORDER
			);
			id_ > Database::CompressionAlgorithmMax)
		{
			this->raiseError(
				"Unsupported compression algorithm"
			);
		}
		else
		{
			this->db->setCompressionAlgo(
				static_cast<Database::CompressionAlgorithm>(id_)
			);
		}
	}
}

void KeePass2Reader::setMasterSeed(
	const QByteArray &data
)
{
	if(data.size() != 32)
	{
		this->raiseError(
			"Invalid master seed size"
		);
	}
	else
	{
		this->masterSeed = data;
	}
}

void KeePass2Reader::setTransformSeed(
	const QByteArray &data
)
{
	if(data.size() != 32)
	{
		this->raiseError(
			"Invalid transform seed size"
		);
	}
	else
	{
		this->transformSeed = data;
	}
}

void KeePass2Reader::setTansformRounds(
	const QByteArray &data
)
{
	if(data.size() != 8)
	{
		this->raiseError(
			"Invalid transform rounds size"
		);
	}
	else
	{
		if(!this->db->setTransformRounds(
			Endian::bytesToUInt64(
				data,
				KeePass2::BYTEORDER
			)
		))
		{
			this->raiseError(
				this->tr(
					"Unable to calculate master key"
				)
			);
		}
	}
}

void KeePass2Reader::setEncryptionIV(
	const QByteArray &data
)
{
	if(data.size() != 16)
	{
		this->raiseError(
			"Invalid encryption iv size"
		);
	}
	else
	{
		this->encryptionIV = data;
	}
}

void KeePass2Reader::setProtectedStreamKey(
	const QByteArray &data
)
{
	if(data.size() != 32)
	{
		this->raiseError(
			"Invalid stream key size"
		);
	}
	else
	{
		this->protectedStreamKey = data;
	}
}

void KeePass2Reader::setStreamStartBytes(
	const QByteArray &data
)
{
	if(data.size() != 32)
	{
		this->raiseError(
			"Invalid start bytes size"
		);
	}
	else
	{
		this->streamStartBytes = data;
	}
}

void KeePass2Reader::setInnerRandomStreamID(
	const QByteArray &data
)
{
	if(data.size() != 4)
	{
		this->raiseError(
			"Invalid random stream id size"
		);
	}
	else
	{
		if(const quint32 id_ = Endian::bytesToUInt32(
				data,
				KeePass2::BYTEORDER
			);
			id_ != KeePass2::Salsa20)
		{
			this->raiseError(
				"Unsupported random stream algorithm"
			);
		}
	}
}
