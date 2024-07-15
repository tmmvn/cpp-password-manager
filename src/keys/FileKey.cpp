/*
*  Copyright (C) 2011 Felix Geyer <debfx@fobos.de>
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
#include "FileKey.h"
#include <QFile>
#include <QXmlStreamReader>
#include "core/Tools.h"
#include "crypto/CryptoHash.h"
#include "crypto/Random.h"

FileKey::FileKey()
{
}

bool FileKey::load(
	QIODevice* device
)
{
	// we may need to read the file multiple times
	if(device->isSequential())
	{
		return false;
	}
	if(device->size() == 0)
	{
		return false;
	}
	// try different key file formats
	if(!device->reset())
	{
		return false;
	}
	if(this->loadXml(
		device
	))
	{
		return true;
	}
	if(!device->reset())
	{
		return false;
	}
	if(this->loadBinary(
		device
	))
	{
		return true;
	}
	if(!device->reset())
	{
		return false;
	}
	if(this->loadHex(
		device
	))
	{
		return true;
	}
	if(!device->reset())
	{
		return false;
	}
	if(this->loadHashed(
		device
	))
	{
		return true;
	}
	return false;
}

bool FileKey::load(
	const QString &fileName,
	QString* errorMsg
)
{
	QFile file_(
		fileName
	);
	if(!file_.open(
		QFile::ReadOnly
	))
	{
		if(errorMsg)
		{
			*errorMsg = file_.errorString();
		}
		return false;
	}
	bool result_ = load(
		&file_
	);
	file_.close();
	if(file_.error())
	{
		result_ = false;
		if(errorMsg)
		{
			*errorMsg = file_.errorString();
		}
	}
	return result_;
}

QByteArray FileKey::rawKey() const
{
	return this->key;
}

FileKey* FileKey::clone() const
{
	return new FileKey(
		*this
	);
}

void FileKey::create(
	QIODevice* device
)
{
	QXmlStreamWriter xmlWriter_(
		device
	);
	xmlWriter_.writeStartDocument(
		"1.0"
	);
	xmlWriter_.writeStartElement(
		"KeyFile"
	);
	xmlWriter_.writeStartElement(
		"Meta"
	);
	xmlWriter_.writeTextElement(
		"Version",
		"1.00"
	);
	xmlWriter_.writeEndElement();
	xmlWriter_.writeStartElement(
		"Key"
	);
	const QByteArray data_ = Random::getInstance()->getRandomArray(
		32
	);
	xmlWriter_.writeTextElement(
		"Data",
		QString::fromLatin1(
			data_.toBase64()
		)
	);
	xmlWriter_.writeEndElement();
	xmlWriter_.writeEndDocument();
}

bool FileKey::create(
	const QString &fileName,
	QString* errorMsg
)
{
	QFile file_(
		fileName
	);
	if(!file_.open(
		QFile::WriteOnly
	))
	{
		if(errorMsg)
		{
			*errorMsg = file_.errorString();
		}
		return false;
	}
	create(
		&file_
	);
	file_.close();
	if(file_.error())
	{
		if(errorMsg)
		{
			*errorMsg = file_.errorString();
		}
		return false;
	}
	return true;
}

bool FileKey::loadXml(
	QIODevice* device
)
{
	QXmlStreamReader xmlReader_(
		device
	);
	if(!xmlReader_.error() && xmlReader_.readNextStartElement())
	{
		if(xmlReader_.name().toString() != "KeyFile")
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	auto correctMeta_ = false;
	QByteArray data_;
	while(!xmlReader_.error() && xmlReader_.readNextStartElement())
	{
		if(xmlReader_.name().toString() == "Meta")
		{
			correctMeta_ = loadXmlMeta(
				xmlReader_
			);
		}
		else if(xmlReader_.name().toString() == "Key")
		{
			data_ = loadXmlKey(
				xmlReader_
			);
		}
	}
	if(!xmlReader_.error() && correctMeta_ && !data_.isEmpty())
	{
		key = data_;
		return true;
	}
	return false;
}

bool FileKey::loadXmlMeta(
	QXmlStreamReader &xmlReader
)
{
	auto corectVersion_ = false;
	while(!xmlReader.error() && xmlReader.readNextStartElement())
	{
		if(xmlReader.name().toString() == "Version")
		{
			// TODO: error message about incompatible key file version
			if(xmlReader.readElementText() == "1.00")
			{
				corectVersion_ = true;
			}
		}
	}
	return corectVersion_;
}

QByteArray FileKey::loadXmlKey(
	QXmlStreamReader &xmlReader
)
{
	QByteArray data_;
	while(!xmlReader.error() && xmlReader.readNextStartElement())
	{
		if(xmlReader.name().toString() == "Data")
		{
			// TODO: do we need to enforce a specific data.size()?
			if(QByteArray rawData_ = xmlReader.readElementText().toLatin1();
				Tools::isBase64(
					rawData_
				))
			{
				data_ = QByteArray::fromBase64(
					rawData_
				);
			}
		}
	}
	return data_;
}

bool FileKey::loadBinary(
	QIODevice* device
)
{
	if(device->size() != 32)
	{
		return false;
	}
	if(QByteArray data_;
		!Tools::readAllFromDevice(
			device,
			data_
		) || data_.size() != 32)
	{
		return false;
	}
	else
	{
		this->key = data_;
		return true;
	}
}

bool FileKey::loadHex(
	QIODevice* device
)
{
	if(device->size() != 64)
	{
		return false;
	}
	QByteArray data_;
	if(!Tools::readAllFromDevice(
		device,
		data_
	) || data_.size() != 64)
	{
		return false;
	}
	if(!Tools::isHex(
		data_
	))
	{
		return false;
	}
	const QByteArray key_ = QByteArray::fromHex(
		data_
	);
	if(key_.size() != 32)
	{
		return false;
	}
	this->key = key_;
	return true;
}

bool FileKey::loadHashed(
	QIODevice* device
)
{
	CryptoHash cryptoHash_(
		CryptoHash::Sha256
	);
	QByteArray buffer_;
	do
	{
		if(!Tools::readFromDevice(
			device,
			buffer_
		))
		{
			return false;
		}
		cryptoHash_.addData(
			buffer_
		);
	}
	while(!buffer_.isEmpty());
	this->key = cryptoHash_.getResult();
	return true;
}
