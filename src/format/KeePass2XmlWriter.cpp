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
#include "KeePass2XmlWriter.h"
#include <QBuffer>
#include <QFile>
#include "core/Metadata.h"
#include "format/KeePass2RandomStream.h"
#include "streams/QtIOCompressor"

KeePass2XmlWriter::KeePass2XmlWriter()
	: db(
		nullptr
	),
	meta(
		nullptr
	),
	randomStream(
		nullptr
	),
	error(
		false
	)
{
	this->xml.setAutoFormatting(
		true
	);
	this->xml.setAutoFormattingIndent(
		-1
	); // 1 tab
}

void KeePass2XmlWriter::writeDatabase(
	QIODevice* device,
	Database* db,
	KeePass2RandomStream* randomStream,
	const QByteArray &headerHash
)
{
	if(db == nullptr)
	{
		qWarning() << "No database";
		return;
	}
	if(device == nullptr)
	{
		qWarning() << "No device";
		return;
	}
	this->db = db;
	this->meta = db->getMetadata();
	this->randomStream = randomStream;
	this->headerHash = headerHash;
	this->generateIdMap();
	this->xml.setDevice(
		device
	);
	this->xml.writeStartDocument(
		"1.0",
		true
	);
	this->xml.writeStartElement(
		"KeePassFile"
	);
	this->writeMetadata();
	this->writeRoot();
	this->xml.writeEndElement();
	this->xml.writeEndDocument();
	if(this->xml.hasError())
	{
		this->raiseError(
			device->errorString()
		);
	}
}

void KeePass2XmlWriter::writeDatabase(
	const QString &filename,
	Database* db
)
{
	QFile file_(
		filename
	);
	file_.open(
		QIODevice::WriteOnly | QIODevice::Truncate
	);
	this->writeDatabase(
		&file_,
		db,
		nullptr,
		QByteArray()
	);
}

bool KeePass2XmlWriter::hasError() const
{
	return this->error;
}

QString KeePass2XmlWriter::getErrorString()
{
	return this->errorStr;
}

void KeePass2XmlWriter::generateIdMap()
{
	const QList<Entry*> allEntries_ = this->db->getRootGroup()->
		getEntriesRecursive(
			true
		);
	auto nextId_ = 0;
	for(Entry* entry_: allEntries_)
	{
		const QList<QString> attachmentKeys_ = entry_->getAttachments()->
			getKeys();
		for(const QString &key_: attachmentKeys_)
		{
			if(QByteArray data_ = entry_->getAttachments()->getValue(
					key_
				);
				!this->idMap.contains(
					data_
				))
			{
				this->idMap.insert(
					data_,
					nextId_++
				);
			}
		}
	}
}

void KeePass2XmlWriter::writeMetadata()
{
	this->xml.writeStartElement(
		"Meta"
	);
	this->writeString(
		"Generator",
		this->meta->getGenerator()
	);
	if(!this->headerHash.isEmpty())
	{
		this->writeBinary(
			"HeaderHash",
			this->headerHash
		);
	}
	this->writeString(
		"DatabaseName",
		this->meta->getName()
	);
	this->writeDateTime(
		"DatabaseNameChanged",
		this->meta->getNameChangedTime()
	);
	this->writeString(
		"DatabaseDescription",
		this->meta->getDescription()
	);
	this->writeDateTime(
		"DatabaseDescriptionChanged",
		this->meta->getDescriptionTimeChanged()
	);
	this->writeString(
		"DefaultUserName",
		this->meta->getDefaultUserName()
	);
	this->writeDateTime(
		"DefaultUserNameChanged",
		this->meta->getDefaultUserNameChanged()
	);
	this->writeNumber(
		"MaintenanceHistoryDays",
		this->meta->getMaintenanceHistoryDays()
	);
	this->writeColor(
		"Color",
		this->meta->getColor()
	);
	this->writeDateTime(
		"MasterKeyChanged",
		this->meta->getMasterKeyChanged()
	);
	this->writeNumber(
		"MasterKeyChangeRec",
		this->meta->masterKeyChangeRec()
	);
	this->writeNumber(
		"MasterKeyChangeForce",
		this->meta->masterKeyChangeForce()
	);
	this->writeMemoryProtection();
	this->writeCustomIcons();
	this->writeBool(
		"RecycleBinEnabled",
		this->meta->recycleBinEnabled()
	);
	this->writeUUID(
		"RecycleBinUUID",
		this->meta->getRecycleBin()
	);
	this->writeDateTime(
		"RecycleBinChanged",
		this->meta->getRecycleBinChangedTime()
	);
	this->writeUUID(
		"EntryTemplatesGroup",
		this->meta->getEntryTemplatesGroup()
	);
	this->writeDateTime(
		"EntryTemplatesGroupChanged",
		this->meta->getEntryTemplatesGroupChangedTime()
	);
	this->writeUUID(
		"LastSelectedGroup",
		this->meta->getLastSelectedGroup()
	);
	this->writeUUID(
		"LastTopVisibleGroup",
		this->meta->getLastTopVisibleGroup()
	);
	this->writeNumber(
		"HistoryMaxItems",
		this->meta->getHistoryMaxItems()
	);
	this->writeNumber(
		"HistoryMaxSize",
		this->meta->getHistoryMaxSize()
	);
	this->writeBinaries();
	this->writeCustomData();
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeMemoryProtection()
{
	this->xml.writeStartElement(
		"MemoryProtection"
	);
	this->writeBool(
		"ProtectTitle",
		this->meta->protectTitle()
	);
	this->writeBool(
		"ProtectUserName",
		this->meta->protectUsername()
	);
	this->writeBool(
		"ProtectPassword",
		this->meta->protectPassword()
	);
	this->writeBool(
		"ProtectURL",
		this->meta->protectUrl()
	);
	this->writeBool(
		"ProtectNotes",
		this->meta->protectNotes()
	);
	// writeBool("AutoEnableVisualHiding", m_meta->autoEnableVisualHiding());
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeCustomIcons()
{
	this->xml.writeStartElement(
		"CustomIcons"
	);
	const QList<UUID> customIconsOrder_ = this->meta->getCustomIconsOrder();
	for(const UUID &uuid_: customIconsOrder_)
	{
		this->writeIcon(
			uuid_,
			this->meta->getCustomIcon(
				uuid_
			)
		);
	}
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeIcon(
	const UUID &uuid,
	const QImage &icon
)
{
	this->xml.writeStartElement(
		"Icon"
	);
	this->writeUUID(
		"UUID",
		uuid
	);
	QByteArray ba_;
	QBuffer buffer_(
		&ba_
	);
	buffer_.open(
		QIODevice::WriteOnly
	);
	// TODO: check !icon.save()
	icon.save(
		&buffer_,
		"PNG"
	);
	buffer_.close();
	this->writeBinary(
		"Data",
		ba_
	);
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeBinaries()
{
	this->xml.writeStartElement(
		"Binaries"
	);
	for(QHash<QByteArray, int>::const_iterator i_ = this->idMap.constBegin(); i_
		!= this->idMap.constEnd(); ++i_)
	{
		this->xml.writeStartElement(
			"Binary"
		);
		this->xml.writeAttribute(
			"ID",
			QString::number(
				i_.value()
			)
		);
		QByteArray data_;
		if(this->db->getCompressionAlgo() == Database::CompressionGZip)
		{
			this->xml.writeAttribute(
				"Compressed",
				"True"
			);
			QBuffer buffer_;
			buffer_.open(
				QIODevice::ReadWrite
			);
			QtIOCompressor compressor_(
				&buffer_
			);
			compressor_.setStreamFormat(
				QtIOCompressor::GzipFormat
			);
			compressor_.open(
				QIODevice::WriteOnly
			);
			if(const qint64 bytesWritten_ = compressor_.write(
					i_.key()
				);
				bytesWritten_ != i_.key().size())
			{
				this->error = true;
				this->errorStr = "Compression error";
				return;
			}
			compressor_.close();
			buffer_.seek(
				0
			);
			data_ = buffer_.readAll();
		}
		else
		{
			data_ = i_.key();
		}
		if(!data_.isEmpty())
		{
			this->xml.writeCharacters(
				QString::fromLatin1(
					data_.toBase64()
				)
			);
		}
		this->xml.writeEndElement();
	}
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeCustomData()
{
	this->xml.writeStartElement(
		"CustomData"
	);
	const QHash<QString, QString> customFields_ = this->meta->getCustomFields();
	const QList<QString> keyList_ = customFields_.keys();
	for(const QString &key_: keyList_)
	{
		this->writeCustomDataItem(
			key_,
			customFields_.value(
				key_
			)
		);
	}
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeCustomDataItem(
	const QString &key,
	const QString &value
)
{
	this->xml.writeStartElement(
		"Item"
	);
	this->writeString(
		"Key",
		key
	);
	this->writeString(
		"Value",
		value
	);
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeRoot()
{
	if(this->db->getRootGroup() == nullptr)
	{
		qWarning() << "No root group";
		return;
	}
	this->xml.writeStartElement(
		"Root"
	);
	this->writeGroup(
		this->db->getRootGroup()
	);
	this->writeDeletedObjects();
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeGroup(
	const Group* group
)
{
	if(group->getUUID().isNull())
	{
		qWarning() << "No group UUID";
		return;
	}
	this->xml.writeStartElement(
		"Group"
	);
	this->writeUUID(
		"UUID",
		group->getUUID()
	);
	this->writeString(
		"Name",
		group->getName()
	);
	this->writeString(
		"Notes",
		group->getNotes()
	);
	this->writeNumber(
		"IconID",
		group->getIconNumber()
	);
	if(!group->getIconUUID().isNull())
	{
		this->writeUUID(
			"CustomIconUUID",
			group->getIconUUID()
		);
	}
	this->writeTimes(
		group->getTimeInfo()
	);
	this->writeBool(
		"IsExpanded",
		group->isExpanded()
	);
	this->writeTriState(
		"EnableSearching",
		group->isSearchingEnabled()
	);
	this->writeUUID(
		"LastTopVisibleEntry",
		group->getLastTopVisibleEntry()
	);
	const QList<Entry*> &entryList_ = group->getEntries();
	for(const Entry* entry_: entryList_)
	{
		this->writeEntry(
			entry_
		);
	}
	const QList<Group*> &children_ = group->getChildren();
	for(const Group* child_: children_)
	{
		this->writeGroup(
			child_
		);
	}
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeTimes(
	const TimeInfo &ti
)
{
	this->xml.writeStartElement(
		"Times"
	);
	this->writeDateTime(
		"LastModificationTime",
		ti.getLastModificationTime()
	);
	this->writeDateTime(
		"CreationTime",
		ti.getCreationTime()
	);
	this->writeDateTime(
		"LastAccessTime",
		ti.getLastAccessTime()
	);
	this->writeDateTime(
		"ExpiryTime",
		ti.getExpiryTime()
	);
	this->writeBool(
		"Expires",
		ti.getExpires()
	);
	this->writeNumber(
		"UsageCount",
		ti.getUsageCount()
	);
	this->writeDateTime(
		"LocationChanged",
		ti.getLocationChanged()
	);
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeDeletedObjects()
{
	this->xml.writeStartElement(
		"DeletedObjects"
	);
	const QList<DeletedObject> delObjList_ = this->db->getDeletedObjects();
	for(const DeletedObject &delObj_: delObjList_)
	{
		this->writeDeletedObject(
			delObj_
		);
	}
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeDeletedObject(
	const DeletedObject &delObj
)
{
	this->xml.writeStartElement(
		"DeletedObject"
	);
	this->writeUUID(
		"UUID",
		delObj.uuid
	);
	this->writeDateTime(
		"DeletionTime",
		delObj.deletionTime
	);
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeEntry(
	const Entry* entry
)
{
	if(entry->getUUID().isNull())
	{
		qWarning() << "No UUID for entry";
		return;
	}
	this->xml.writeStartElement(
		"Entry"
	);
	this->writeUUID(
		"UUID",
		entry->getUUID()
	);
	this->writeNumber(
		"IconID",
		entry->getIconNumber()
	);
	if(!entry->getIconUUID().isNull())
	{
		this->writeUUID(
			"CustomIconUUID",
			entry->getIconUUID()
		);
	}
	this->writeColor(
		"ForegroundColor",
		entry->getForegroundColor()
	);
	this->writeColor(
		"BackgroundColor",
		entry->getBackgroundColor()
	);
	this->writeString(
		"OverrideURL",
		entry->getOverrideURL()
	);
	this->writeString(
		"Tags",
		entry->getTags()
	);
	this->writeTimes(
		entry->getTimeInfo()
	);
	const QList<QString> attributesKeyList_ = entry->getAttributes()->getKeys();
	for(const QString &key_: attributesKeyList_)
	{
		this->xml.writeStartElement(
			"String"
		);
		const bool protect_ = (key_ == "Title" && this->meta->protectTitle()) ||
			(key_ == "UserName" && this->meta->protectUsername()) || (key_ ==
				"Password" && this->meta->protectPassword()) || (key_ == "URL"
				&& this->meta->protectUrl()) || (key_ == "Notes" && this->meta->
				protectNotes()) || entry->getAttributes()->isProtected(
				key_
			);
		this->writeString(
			"Key",
			key_
		);
		this->xml.writeStartElement(
			"Value"
		);
		QString value_;
		if(protect_)
		{
			if(this->randomStream)
			{
				this->xml.writeAttribute(
					"Protected",
					"True"
				);
				bool ok_;
				QByteArray rawData_ = this->randomStream->process(
					entry->getAttributes()->getValue(
						key_
					).toUtf8(),
					&ok_
				);
				if(!ok_)
				{
					this->raiseError(
						this->randomStream->getErrorString()
					);
				}
				value_ = QString::fromLatin1(
					rawData_.toBase64()
				);
			}
			else
			{
				this->xml.writeAttribute(
					"ProtectInMemory",
					"True"
				);
				value_ = entry->getAttributes()->getValue(
					key_
				);
			}
		}
		else
		{
			value_ = entry->getAttributes()->getValue(
				key_
			);
		}
		if(!value_.isEmpty())
		{
			this->xml.writeCharacters(
				stripInvalidXml10Chars(
					value_
				)
			);
		}
		this->xml.writeEndElement();
		this->xml.writeEndElement();
	}
	const QList<QString> attachmentsKeyList_ = entry->getAttachments()->
		getKeys();
	for(const QString &key_: attachmentsKeyList_)
	{
		this->xml.writeStartElement(
			"Binary"
		);
		this->writeString(
			"Key",
			key_
		);
		this->xml.writeStartElement(
			"Value"
		);
		this->xml.writeAttribute(
			"Ref",
			QString::number(
				this->idMap[entry->getAttachments()->getValue(
					key_
				)]
			)
		);
		this->xml.writeEndElement();
		this->xml.writeEndElement();
	}
	// write history only for entries that are not history items
	if(entry->parent())
	{
		this->writeEntryHistory(
			entry
		);
	}
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeEntryHistory(
	const Entry* entry
)
{
	this->xml.writeStartElement(
		"History"
	);
	const QList<Entry*> &historyItems_ = entry->getHistoryItems();
	for(const Entry* item_: historyItems_)
	{
		this->writeEntry(
			item_
		);
	}
	this->xml.writeEndElement();
}

void KeePass2XmlWriter::writeString(
	const QString &qualifiedName,
	const QString &string
)
{
	if(string.isEmpty())
	{
		this->xml.writeEmptyElement(
			qualifiedName
		);
	}
	else
	{
		this->xml.writeTextElement(
			qualifiedName,
			this->stripInvalidXml10Chars(
				string
			)
		);
	}
}

void KeePass2XmlWriter::writeNumber(
	const QString &qualifiedName,
	const int number
)
{
	this->writeString(
		qualifiedName,
		QString::number(
			number
		)
	);
}

void KeePass2XmlWriter::writeBool(
	const QString &qualifiedName,
	const bool b
)
{
	if(b)
	{
		this->writeString(
			qualifiedName,
			"True"
		);
	}
	else
	{
		this->writeString(
			qualifiedName,
			"False"
		);
	}
}

void KeePass2XmlWriter::writeDateTime(
	const QString &qualifiedName,
	const QDateTime &dateTime
)
{
	if(!dateTime.isValid())
	{
		qWarning() << "Date time is not valid";
		return;
	}
	if(dateTime.timeSpec() != Qt::UTC)
	{
		qWarning() << "Wrong spec for date time";
		return;
	}
	QString dateTimeStr_ = dateTime.toString(
		Qt::ISODate
	);
	// Qt < 4.8 doesn't append a 'Z' at the end
	if(!dateTimeStr_.isEmpty() && dateTimeStr_[dateTimeStr_.size() - 1] != 'Z')
	{
		dateTimeStr_.append(
			'Z'
		);
	}
	this->writeString(
		qualifiedName,
		dateTimeStr_
	);
}

void KeePass2XmlWriter::writeUUID(
	const QString &qualifiedName,
	const UUID &uuid
)
{
	this->writeString(
		qualifiedName,
		uuid.toBase64()
	);
}

void KeePass2XmlWriter::writeUUID(
	const QString &qualifiedName,
	const Group* group
)
{
	if(group)
	{
		this->writeUUID(
			qualifiedName,
			group->getUUID()
		);
	}
	else
	{
		this->writeUUID(
			qualifiedName,
			UUID()
		);
	}
}

void KeePass2XmlWriter::writeUUID(
	const QString &qualifiedName,
	const Entry* entry
)
{
	if(entry)
	{
		this->writeUUID(
			qualifiedName,
			entry->getUUID()
		);
	}
	else
	{
		this->writeUUID(
			qualifiedName,
			UUID()
		);
	}
}

void KeePass2XmlWriter::writeBinary(
	const QString &qualifiedName,
	const QByteArray &ba
)
{
	this->writeString(
		qualifiedName,
		QString::fromLatin1(
			ba.toBase64()
		)
	);
}

void KeePass2XmlWriter::writeColor(
	const QString &qualifiedName,
	const QColor &color
)
{
	QString colorStr_;
	if(color.isValid())
	{
		colorStr_ = QString(
			"#%1%2%3"
		).arg(
			colorPartToString(
				color.red()
			)
		).arg(
			colorPartToString(
				color.green()
			)
		).arg(
			colorPartToString(
				color.blue()
			)
		);
	}
	this->writeString(
		qualifiedName,
		colorStr_
	);
}

void KeePass2XmlWriter::writeTriState(
	const QString &qualifiedName,
	const Group::TriState triState
)
{
	QString value_;
	if(triState == Group::Inherit)
	{
		value_ = "null";
	}
	else if(triState == Group::Enable)
	{
		value_ = "true";
	}
	else
	{
		value_ = "false";
	}
	this->writeString(
		qualifiedName,
		value_
	);
}

QString KeePass2XmlWriter::colorPartToString(
	const int value
)
{
	QString str_ = QString::number(
		value,
		16
	).toUpper();
	if(str_.length() == 1)
	{
		str_.prepend(
			"0"
		);
	}
	return str_;
}

QString KeePass2XmlWriter::stripInvalidXml10Chars(
	const QString &str
)
{
	QString result_;
	result_.reserve(
		str.size()
	); // Reserve space to avoid reallocations
	for(auto i_ = 0; i_ < str.size(); ++i_)
	{
		const QChar ch_ = str.at(
			i_
		);
		if(const ushort uc_ = ch_.unicode();
			(uc_ >= 0x20 && uc_ <= 0xD7FF) ||
			// Valid XML 1.0 range (excluding surrogates)
			(uc_ >= 0xE000 && uc_ <= 0xFFFD) ||
			// Valid XML 1.0 range (excluding surrogates)
			uc_ == 0x9 || uc_ == 0xA || uc_ == 0xD)
		{
			// Allowed whitespace
			result_.append(
				ch_
			);
			// Handle potential surrogate pairs
			if(ch_.isHighSurrogate() && i_ < str.size() - 1)
			{
				if(const QChar nextCh_ = str.at(
						i_ + 1
					);
					nextCh_.isLowSurrogate())
				{
					result_.append(
						nextCh_
					);
					++i_; // Skip the low surrogate in the next iteration
				}
			}
		}
		else
		{
			qWarning(
				"Stripping invalid XML 1.0 codepoint %x",
				uc_
			);
		}
	}
	return result_;
}

void KeePass2XmlWriter::raiseError(
	const QString &errorMessage
)
{
	qCritical() << errorMessage;
	this->error = true;
	this->errorStr = errorMessage;
}
