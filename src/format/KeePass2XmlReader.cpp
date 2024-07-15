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
#include "KeePass2XmlReader.h"
#include <QBuffer>
#include <QFile>
#include "core/Database.h"
#include "core/DatabaseIcons.h"
#include "core/Global.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "core/Tools.h"
#include "format/KeePass2RandomStream.h"
#include "streams/QtIOCompressor"
typedef QPair<QString, QString> StringPair;

KeePass2XmlReader::KeePass2XmlReader()
	: randomStream(
		nullptr
	),
	db(
		nullptr
	),
	meta(
		nullptr
	),
	tmpParent(
		nullptr
	),
	error(
		false
	),
	strictMode(
		false
	)
{
}

void KeePass2XmlReader::setStrictMode(
	const bool strictMode
)
{
	this->strictMode = strictMode;
}

void KeePass2XmlReader::readDatabase(
	QIODevice* device,
	Database* db,
	KeePass2RandomStream* randomStream
)
{
	if(db == nullptr)
	{
		qWarning() << "KeePass2XmlReader::readDatabase: db is nullptr";
		return;
	}
	if(device == nullptr)
	{
		qWarning() << "KeePass2XmlReader::readDatabase: device is nullptr";
		return;
	}
	this->error = false;
	this->errorStr.clear();
	this->xml.clear();
	this->xml.setDevice(
		device
	);
	this->db = db;
	this->meta = this->db->getMetadata();
	this->meta->setUpdateDatetime(
		false
	);
	this->randomStream = randomStream;
	this->headerHash.clear();
	this->tmpParent = new Group();
	auto rootGroupParsed_ = false;
	if(this->xml.error())
	{
		this->raiseError(
			"XML error at start."
		);
		return;
	}
	if(this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "KeePassFile")
		{
			rootGroupParsed_ = this->parseKeePassFile();
		}
	}
	if(this->xml.error())
	{
		this->raiseError(
			"XML error after reading root group."
		);
		return;
	}
	if(!rootGroupParsed_)
	{
		this->raiseError(
			"No root group"
		);
		return;
	}
	if(!this->tmpParent->getChildren().isEmpty())
	{
		qWarning(
			"KeePass2XmlReader::readDatabase: found %lld invalid group reference(s)",
			this->tmpParent->getChildren().size()
		);
	}
	if(!this->tmpParent->getEntries().isEmpty())
	{
		qWarning(
			"KeePass2XmlReader::readDatabase: found %lld invalid entry reference(s)",
			this->tmpParent->getChildren().size()
		);
	}
	QStringList unusedKeys_;
	for(const QString &key: this->binaryPool.keys())
	{
		if(!this->binaryMap.contains(
			key
		))
		{
			unusedKeys_.append(
				key
			);
		}
	}
	// Find keys in 'b' but not in 'a'
	QStringList unmappedKeys_;
	for(const QString &key: this->binaryMap.keys())
	{
		if(!this->binaryPool.contains(
			key
		))
		{
			unmappedKeys_.append(
				key
			);
		}
	}
	if(!unmappedKeys_.isEmpty())
	{
		this->raiseError(
			"Unmapped keys left."
		);
		return;
	}
	for(const QString &key_: unusedKeys_)
	{
		qWarning(
			"KeePass2XmlReader::readDatabase: found unused key \"%s\"",
			qPrintable(
				key_
			)
		);
	}
	for(auto i_ = binaryMap.constBegin(); i_ != binaryMap.constEnd(); ++i_)
	{
		const auto &[fst_, snd_] = i_.value();
		fst_->getAttachments()->set(
			snd_,
			binaryPool[i_.key()]
		);
	}
	this->meta->setUpdateDatetime(
		true
	);
	for(QHash<UUID, Group*>::const_iterator iGroup_ = this->groups.constBegin();
		iGroup_ != this->groups.constEnd(); ++iGroup_)
	{
		iGroup_.value()->setUpdateTimeinfo(
			true
		);
	}
	for(QHash<UUID, Entry*>::const_iterator iEntry_ = this->entries.constBegin()
		; iEntry_ != this->entries.constEnd(); ++iEntry_)
	{
		iEntry_.value()->setUpdateTimeinfo(
			true
		);
		const QList<Entry*> historyItems_ = iEntry_.value()->getHistoryItems();
		for(Entry* histEntry_: historyItems_)
		{
			histEntry_->setUpdateTimeinfo(
				true
			);
		}
	}
	delete this->tmpParent;
}

Database* KeePass2XmlReader::readDatabase(
	QIODevice* device
)
{
	const auto db_ = new Database();
	this->readDatabase(
		device,
		db_
	);
	return db_;
}

Database* KeePass2XmlReader::readDatabase(
	const QString &filename
)
{
	QFile file_(
		filename
	);
	file_.open(
		QIODevice::ReadOnly
	);
	return this->readDatabase(
		&file_
	);
}

bool KeePass2XmlReader::hasError() const
{
	return this->error || this->xml.hasError();
}

QString KeePass2XmlReader::getErrorString()
{
	if(this->xml.hasError())
	{
		return QString(
			"XML error:\n%1\nLine %2, column %3"
		).arg(
			this->xml.errorString()
		).arg(
			this->xml.lineNumber()
		).arg(
			this->xml.columnNumber()
		);
	}
	if(this->error)
	{
		return this->errorStr;
	}
	return QString();
}

void KeePass2XmlReader::raiseError(
	const QString &errorMessage
)
{
	qCritical() << errorMessage;
	this->error = true;
	this->errorStr = errorMessage;
}

QByteArray KeePass2XmlReader::getHeaderHash()
{
	return this->headerHash;
}

bool KeePass2XmlReader::parseKeePassFile()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() ==
		"KeePassFile"))
	{
		qWarning() << "Failed parsing KeePassFile";
		return false;
	}
	auto rootElementFound_ = false;
	auto rootParsedSuccesfully_ = false;
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "Meta")
		{
			this->parseMeta();
		}
		else if(this->xml.name().toString() == "Root")
		{
			if(rootElementFound_)
			{
				rootParsedSuccesfully_ = false;
				this->raiseError(
					"Multiple root elements"
				);
			}
			else
			{
				rootParsedSuccesfully_ = this->parseRoot();
				rootElementFound_ = true;
			}
		}
		else
		{
			this->skipCurrentElement();
		}
	}
	return rootParsedSuccesfully_;
}

void KeePass2XmlReader::parseMeta()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() == "Meta"))
	{
		qWarning() << "Failed parsing Meta";
		return;
	}
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "Generator")
		{
			this->meta->setGenerator(
				this->readString()
			);
		}
		else if(this->xml.name().toString() == "HeaderHash")
		{
			this->headerHash = this->readBinary();
		}
		else if(this->xml.name().toString() == "DatabaseName")
		{
			this->meta->setName(
				this->readString()
			);
		}
		else if(this->xml.name().toString() == "DatabaseNameChanged")
		{
			this->meta->setNameChanged(
				this->readDateTime()
			);
		}
		else if(this->xml.name().toString() == "DatabaseDescription")
		{
			this->meta->setDescription(
				this->readString()
			);
		}
		else if(this->xml.name().toString() == "DatabaseDescriptionChanged")
		{
			this->meta->setDescriptionChanged(
				this->readDateTime()
			);
		}
		else if(this->xml.name().toString() == "DefaultUserName")
		{
			this->meta->setDefaultUserName(
				this->readString()
			);
		}
		else if(this->xml.name().toString() == "DefaultUserNameChanged")
		{
			this->meta->setDefaultUserNameChanged(
				this->readDateTime()
			);
		}
		else if(this->xml.name().toString() == "MaintenanceHistoryDays")
		{
			this->meta->setMaintenanceHistoryDays(
				this->readNumber()
			);
		}
		else if(this->xml.name().toString() == "Color")
		{
			this->meta->setColor(
				this->readColor()
			);
		}
		else if(this->xml.name().toString() == "MasterKeyChanged")
		{
			this->meta->setMasterKeyChanged(
				this->readDateTime()
			);
		}
		else if(this->xml.name().toString() == "MasterKeyChangeRec")
		{
			this->meta->setMasterKeyChangeRec(
				this->readNumber()
			);
		}
		else if(this->xml.name().toString() == "MasterKeyChangeForce")
		{
			this->meta->setMasterKeyChangeForce(
				this->readNumber()
			);
		}
		else if(this->xml.name().toString() == "MemoryProtection")
		{
			this->parseMemoryProtection();
		}
		else if(this->xml.name().toString() == "CustomIcons")
		{
			this->parseCustomIcons();
		}
		else if(this->xml.name().toString() == "RecycleBinEnabled")
		{
			this->meta->setRecycleBinEnabled(
				this->readBool()
			);
		}
		else if(this->xml.name().toString() == "RecycleBinUUID")
		{
			this->meta->setRecycleBin(
				this->getGroup(
					this->readUUID()
				)
			);
		}
		else if(this->xml.name().toString() == "RecycleBinChanged")
		{
			this->meta->setRecycleBinChanged(
				this->readDateTime()
			);
		}
		else if(this->xml.name().toString() == "EntryTemplatesGroup")
		{
			this->meta->setEntryTemplatesGroup(
				this->getGroup(
					this->readUUID()
				)
			);
		}
		else if(this->xml.name().toString() == "EntryTemplatesGroupChanged")
		{
			this->meta->setEntryTemplatesGroupChanged(
				this->readDateTime()
			);
		}
		else if(this->xml.name().toString() == "LastSelectedGroup")
		{
			this->meta->setLastSelectedGroup(
				this->getGroup(
					this->readUUID()
				)
			);
		}
		else if(this->xml.name().toString() == "LastTopVisibleGroup")
		{
			this->meta->setLastTopVisibleGroup(
				this->getGroup(
					this->readUUID()
				)
			);
		}
		else if(this->xml.name().toString() == "HistoryMaxItems")
		{
			if(const int value_ = this->readNumber();
				value_ >= -1)
			{
				this->meta->setHistoryMaxItems(
					value_
				);
			}
			else
			{
				this->raiseError(
					"HistoryMaxItems invalid number"
				);
			}
		}
		else if(this->xml.name().toString() == "HistoryMaxSize")
		{
			if(const int value_ = this->readNumber();
				value_ >= -1)
			{
				this->meta->setHistoryMaxSize(
					value_
				);
			}
			else
			{
				this->raiseError(
					"HistoryMaxSize invalid number"
				);
			}
		}
		else if(this->xml.name().toString() == "Binaries")
		{
			this->parseBinaries();
		}
		else if(this->xml.name().toString() == "CustomData")
		{
			this->parseCustomData();
		}
		else
		{
			this->skipCurrentElement();
		}
	}
}

void KeePass2XmlReader::parseMemoryProtection()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() ==
		"MemoryProtection"))
	{
		qWarning() << "Failed parsing MemoryProtection";
		return;
	}
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "ProtectTitle")
		{
			this->meta->setProtectTitle(
				this->readBool()
			);
		}
		else if(this->xml.name().toString() == "ProtectUserName")
		{
			this->meta->setProtectUsername(
				this->readBool()
			);
		}
		else if(this->xml.name().toString() == "ProtectPassword")
		{
			this->meta->setProtectPassword(
				this->readBool()
			);
		}
		else if(this->xml.name().toString() == "ProtectURL")
		{
			this->meta->setProtectUrl(
				this->readBool()
			);
		}
		else if(this->xml.name().toString() == "ProtectNotes")
		{
			this->meta->setProtectNotes(
				this->readBool()
			);
		}
		/*else if (m_xml.name() == "AutoEnableVisualHiding") {
			m_meta->setAutoEnableVisualHiding(readBool());
		}*/
		else
		{
			this->skipCurrentElement();
		}
	}
}

void KeePass2XmlReader::parseCustomIcons()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() ==
		"CustomIcons"))
	{
		qWarning() << "Failed parsing CustomIcons";
		return;
	}
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "Icon")
		{
			this->parseIcon();
		}
		else
		{
			this->skipCurrentElement();
		}
	}
}

void KeePass2XmlReader::parseIcon()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() == "Icon"))
	{
		qWarning() << "Failed parsing Icon";
		return;
	}
	UUID uuid_;
	QImage icon_;
	auto uuidSet_ = false;
	auto iconSet_ = false;
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "UUID")
		{
			uuid_ = this->readUUID();
			uuidSet_ = !uuid_.isNull();
		}
		else if(this->xml.name().toString() == "Data")
		{
			icon_.loadFromData(
				this->readBinary()
			);
			iconSet_ = true;
		}
		else
		{
			this->skipCurrentElement();
		}
	}
	if(uuidSet_ && iconSet_)
	{
		this->meta->addCustomIcon(
			uuid_,
			icon_
		);
	}
	else
	{
		this->raiseError(
			"Missing icon uuid or data"
		);
	}
}

void KeePass2XmlReader::parseBinaries()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() ==
		"Binaries"))
	{
		qWarning() << "Failed parsing Binaries";
		return;
	}
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "Binary")
		{
			QXmlStreamAttributes attr_ = this->xml.attributes();
			QString id_ = attr_.value(
				"ID"
			).toString();
			QByteArray data_;
			if(attr_.value(
				"Compressed"
			).compare(
				QLatin1String(
					"True"
				),
				Qt::CaseInsensitive
			) == 0)
			{
				data_ = readCompressedBinary();
			}
			else
			{
				data_ = readBinary();
			}
			if(this->binaryPool.contains(
				id_
			))
			{
				qWarning(
					"KeePass2XmlReader::parseBinaries: overwriting binary item \"%s\"",
					qPrintable(
						id_
					)
				);
			}
			this->binaryPool.insert(
				id_,
				data_
			);
		}
		else
		{
			this->skipCurrentElement();
		}
	}
}

void KeePass2XmlReader::parseCustomData()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() ==
		"CustomData"))
	{
		qWarning() << "Failed parsing CustomData";
		return;
	}
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "Item")
		{
			this->parseCustomDataItem();
		}
		else
		{
			this->skipCurrentElement();
		}
	}
}

void KeePass2XmlReader::parseCustomDataItem()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() == "Item"))
	{
		qWarning() << "Failed parsing Item";
		return;
	}
	QString key_;
	QString value_;
	auto keySet_ = false;
	auto valueSet_ = false;
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "Key")
		{
			key_ = this->readString();
			keySet_ = true;
		}
		else if(this->xml.name().toString() == "Value")
		{
			value_ = this->readString();
			valueSet_ = true;
		}
		else
		{
			this->skipCurrentElement();
		}
	}
	if(keySet_ && valueSet_)
	{
		this->meta->addCustomField(
			key_,
			value_
		);
	}
	else
	{
		this->raiseError(
			"Missing custom data key or value"
		);
	}
}

bool KeePass2XmlReader::parseRoot()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() == "Root"))
	{
		qWarning() << "Failed parsing Root";
		return false;
	}
	auto groupElementFound_ = false;
	auto groupParsedSuccesfully_ = false;
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "Group")
		{
			if(groupElementFound_)
			{
				groupParsedSuccesfully_ = false;
				this->raiseError(
					"Multiple group elements"
				);
				continue;
			}
			if(Group* rootGroup_ = this->parseGroup())
			{
				const Group* oldRoot_ = this->db->getRootGroup();
				this->db->setRootGroup(
					rootGroup_
				);
				delete oldRoot_;
				groupParsedSuccesfully_ = true;
			}
			groupElementFound_ = true;
		}
		else if(this->xml.name().toString() == "DeletedObjects")
		{
			this->parseDeletedObjects();
		}
		else
		{
			this->skipCurrentElement();
		}
	}
	return groupParsedSuccesfully_;
}

Group* KeePass2XmlReader::parseGroup()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() == "Group"))
	{
		qWarning() << "Failed parsing Group";
		return nullptr;
	}
	auto group_ = new Group();
	group_->setUpdateTimeinfo(
		false
	);
	QList<Group*> children_;
	QList<Entry*> entries_;
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "UUID")
		{
			if(UUID uuid_ = this->readUUID();
				uuid_.isNull())
			{
				if(this->strictMode)
				{
					this->raiseError(
						"Null group uuid"
					);
				}
				else
				{
					group_->setUuid(
						UUID::random()
					);
				}
			}
			else
			{
				group_->setUuid(
					uuid_
				);
			}
		}
		else if(this->xml.name().toString() == "Name")
		{
			group_->setName(
				this->readString()
			);
		}
		else if(this->xml.name().toString() == "Notes")
		{
			group_->setNotes(
				this->readString()
			);
		}
		else if(this->xml.name().toString() == "IconID")
		{
			if(int iconId_ = this->readNumber();
				iconId_ < 0)
			{
				if(this->strictMode)
				{
					this->raiseError(
						"Invalid group icon number"
					);
				}
				iconId_ = 0;
			}
			else
			{
				if(iconId_ >= DatabaseIcons::IconCount)
				{
					qWarning(
						"KeePass2XmlReader::parseGroup: icon id \"%d\" not supported",
						iconId_
					);
				}
				group_->setIcon(
					iconId_
				);
			}
		}
		else if(this->xml.name().toString() == "CustomIconUUID")
		{
			if(UUID uuid_ = this->readUUID();
				!uuid_.isNull())
			{
				group_->setIcon(
					uuid_
				);
			}
		}
		else if(this->xml.name().toString() == "Times")
		{
			group_->setTimeInfo(
				this->parseTimes()
			);
		}
		else if(this->xml.name().toString() == "IsExpanded")
		{
			group_->setExpanded(
				this->readBool()
			);
		}
		else if(this->xml.name().toString() == "EnableSearching")
		{
			if(QString str_ = this->readString();
				str_.compare(
					"null",
					Qt::CaseInsensitive
				) == 0)
			{
				group_->setSearchingEnabled(
					Group::Inherit
				);
			}
			else if(str_.compare(
				"true",
				Qt::CaseInsensitive
			) == 0)
			{
				group_->setSearchingEnabled(
					Group::Enable
				);
			}
			else if(str_.compare(
				"false",
				Qt::CaseInsensitive
			) == 0)
			{
				group_->setSearchingEnabled(
					Group::Disable
				);
			}
			else
			{
				this->raiseError(
					"Invalid EnableSearching value"
				);
			}
		}
		else if(this->xml.name().toString() == "LastTopVisibleEntry")
		{
			group_->setLastTopVisibleEntry(
				this->getEntry(
					this->readUUID()
				)
			);
		}
		else if(this->xml.name().toString() == "Group")
		{
			if(Group* newGroup_ = this->parseGroup())
			{
				children_.append(
					newGroup_
				);
			}
		}
		else if(this->xml.name().toString() == "Entry")
		{
			if(Entry* newEntry_ = parseEntry(
				false
			))
			{
				entries_.append(
					newEntry_
				);
			}
		}
		else
		{
			this->skipCurrentElement();
		}
	}
	if(group_->getUUID().isNull() && !this->strictMode)
	{
		group_->setUuid(
			UUID::random()
		);
	}
	if(!group_->getUUID().isNull())
	{
		const Group* tmpGroup_ = group_;
		group_ = this->getGroup(
			tmpGroup_->getUUID()
		);
		group_->copyDataFrom(
			tmpGroup_
		);
		group_->setUpdateTimeinfo(
			false
		);
		delete tmpGroup_;
	}
	else if(!this->hasError())
	{
		this->raiseError(
			"No group uuid found"
		);
	}
	for(Group* child_: asConst(
			children_
		))
	{
		child_->setParent(
			group_
		);
	}
	for(Entry* entry_: asConst(
			entries_
		))
	{
		entry_->setGroup(
			group_
		);
	}
	return group_;
}

void KeePass2XmlReader::parseDeletedObjects()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() ==
		"DeletedObjects"))
	{
		qWarning() << "Failed parsing DeletedObjects";
		return;
	}
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "DeletedObject")
		{
			this->parseDeletedObject();
		}
		else
		{
			this->skipCurrentElement();
		}
	}
}

void KeePass2XmlReader::parseDeletedObject()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() ==
		"DeletedObject"))
	{
		qWarning() << "Failed parsing DeletedObject";
		return;
	}
	DeletedObject delObj_;
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "UUID")
		{
			if(UUID uuid_ = this->readUUID();
				uuid_.isNull())
			{
				if(this->strictMode)
				{
					this->raiseError(
						"Null DeleteObject uuid"
					);
				}
			}
			else
			{
				delObj_.uuid = uuid_;
			}
		}
		else if(this->xml.name().toString() == "DeletionTime")
		{
			delObj_.deletionTime = this->readDateTime();
		}
		else
		{
			this->skipCurrentElement();
		}
	}
	if(!delObj_.uuid.isNull() && !delObj_.deletionTime.isNull())
	{
		this->db->addDeletedObject(
			delObj_
		);
	}
	else if(this->strictMode)
	{
		this->raiseError(
			"Missing DeletedObject uuid or time"
		);
	}
}

Entry* KeePass2XmlReader::parseEntry(
	const bool history
)
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() == "Entry"))
	{
		qWarning() << "Failed parsing Entry";
		return nullptr;
	}
	auto entry_ = new Entry();
	entry_->setUpdateTimeinfo(
		false
	);
	QList<Entry*> historyItems_;
	QList<StringPair> binaryRefs_;
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "UUID")
		{
			if(UUID uuid_ = this->readUUID();
				uuid_.isNull())
			{
				if(this->strictMode)
				{
					raiseError(
						"Null entry uuid"
					);
				}
				else
				{
					entry_->setUUID(
						UUID::random()
					);
				}
			}
			else
			{
				entry_->setUUID(
					uuid_
				);
			}
		}
		else if(this->xml.name().toString() == "IconID")
		{
			if(int iconId_ = this->readNumber();
				iconId_ < 0)
			{
				if(this->strictMode)
				{
					this->raiseError(
						"Invalid entry icon number"
					);
				}
				iconId_ = 0;
			}
			else
			{
				entry_->setIcon(
					iconId_
				);
			}
		}
		else if(this->xml.name().toString() == "CustomIconUUID")
		{
			if(UUID uuid_ = this->readUUID();
				!uuid_.isNull())
			{
				entry_->setIcon(
					uuid_
				);
			}
		}
		else if(this->xml.name().toString() == "ForegroundColor")
		{
			entry_->setForegroundColor(
				this->readColor()
			);
		}
		else if(this->xml.name().toString() == "BackgroundColor")
		{
			entry_->setBackgroundColor(
				this->readColor()
			);
		}
		else if(this->xml.name().toString() == "OverrideURL")
		{
			entry_->setOverrideURL(
				this->readString()
			);
		}
		else if(this->xml.name().toString() == "Tags")
		{
			entry_->setTags(
				this->readString()
			);
		}
		else if(this->xml.name().toString() == "Times")
		{
			entry_->setTimeInfo(
				this->parseTimes()
			);
		}
		else if(this->xml.name().toString() == "String")
		{
			this->parseEntryString(
				entry_
			);
		}
		else if(this->xml.name().toString() == "Binary")
		{
			if(QPair<QString, QString> ref_ = this->parseEntryBinary(
					entry_
				);
				!ref_.first.isNull() && !ref_.second.isNull())
			{
				binaryRefs_.append(
					ref_
				);
			}
		}
		else if(this->xml.name().toString() == "History")
		{
			if(history)
			{
				this->raiseError(
					"History element in history entry"
				);
			}
			else
			{
				historyItems_ = this->parseEntryHistory();
			}
		}
		else
		{
			this->skipCurrentElement();
		}
	}
	if(entry_->getUUID().isNull() && !this->strictMode)
	{
		entry_->setUUID(
			UUID::random()
		);
	}
	if(!entry_->getUUID().isNull())
	{
		if(history)
		{
			entry_->setUpdateTimeinfo(
				false
			);
		}
		else
		{
			const Entry* tmpEntry_ = entry_;
			entry_ = getEntry(
				tmpEntry_->getUUID()
			);
			entry_->copyDataFrom(
				tmpEntry_
			);
			entry_->setUpdateTimeinfo(
				false
			);
			delete tmpEntry_;
		}
	}
	else if(!this->hasError())
	{
		this->raiseError(
			"No entry uuid found"
		);
	}
	for(Entry* historyItem_: asConst(
			historyItems_
		))
	{
		if(historyItem_->getUUID() != entry_->getUUID())
		{
			if(this->strictMode)
			{
				this->raiseError(
					"History element with different uuid"
				);
			}
			else
			{
				historyItem_->setUUID(
					entry_->getUUID()
				);
			}
		}
		entry_->addHistoryItem(
			historyItem_
		);
	}
	for(const auto &[fst, snd]: asConst(
			binaryRefs_
		))
	{
		// TODO: Replace with QMultiHash
		this->binaryMap.insert(
			fst,
			qMakePair(
				entry_,
				snd
			)
		);
	}
	return entry_;
}

void KeePass2XmlReader::parseEntryString(
	Entry* entry
)
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() == "String"))
	{
		qWarning() << "Failed parsing entry string";
		return;
	}
	QString key_;
	QString value_;
	auto protect_ = false;
	auto keySet_ = false;
	auto valueSet_ = false;
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "Key")
		{
			key_ = this->readString();
			keySet_ = true;
		}
		else if(this->xml.name().toString() == "Value")
		{
			QXmlStreamAttributes attr_ = this->xml.attributes();
			value_ = this->readString();
			const bool isProtected_ = attr_.value(
				"Protected"
			).toString() == "True";
			const bool protectInMemory_ = attr_.value(
				"ProtectInMemory"
			).toString() == "True";
			if(isProtected_ && !value_.isEmpty())
			{
				if(this->randomStream)
				{
					QByteArray ciphertext_ = QByteArray::fromBase64(
						value_.toLatin1()
					);
					bool ok_;
					QByteArray plaintext_ = this->randomStream->process(
						ciphertext_,
						&ok_
					);
					if(!ok_)
					{
						value_.clear();
						this->raiseError(
							this->randomStream->getErrorString()
						);
					}
					else
					{
						value_ = QString::fromUtf8(
							plaintext_
						);
					}
				}
				else
				{
					this->raiseError(
						"Unable to decrypt entry string"
					);
				}
			}
			protect_ = isProtected_ || protectInMemory_;
			valueSet_ = true;
		}
		else
		{
			this->skipCurrentElement();
		}
	}
	if(keySet_ && valueSet_)
	{
		// the default attributes are always there so additionally check if it's empty
		if(entry->getAttributes()->hasKey(
			key_
		) && !entry->getAttributes()->getValue(
			key_
		).isEmpty())
		{
			this->raiseError(
				"Duplicate custom attribute found"
			);
		}
		else
		{
			entry->getAttributes()->set(
				key_,
				value_,
				protect_
			);
		}
	}
	else
	{
		this->raiseError(
			"Entry string key or value missing"
		);
	}
}

QPair<QString, QString> KeePass2XmlReader::parseEntryBinary(
	Entry* entry
)
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() == "Binary"))
	{
		qWarning() << "Failed parsing Binary";
		return qMakePair(
			QString(),
			QString()
		);
	}
	QPair<QString, QString> poolRef_;
	QString key_;
	QByteArray value_;
	auto keySet_ = false;
	auto valueSet_ = false;
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "Key")
		{
			key_ = this->readString();
			keySet_ = true;
		}
		else if(this->xml.name().toString() == "Value")
		{
			if(QXmlStreamAttributes attr_ = this->xml.attributes();
				attr_.hasAttribute(
					"Ref"
				))
			{
				poolRef_ = qMakePair(
					attr_.value(
						"Ref"
					).toString(),
					key_
				);
				this->xml.skipCurrentElement();
			}
			else
			{
				// format compatibility
				value_ = this->readBinary();
				if(const bool isProtected_ = attr_.hasAttribute(
						"Protected"
					) && (attr_.value(
						"Protected"
					).toString() == "True");
					isProtected_ && !value_.isEmpty())
				{
					if(!this->randomStream->processInPlace(
						value_
					))
					{
						this->raiseError(
							this->randomStream->getErrorString()
						);
					}
				}
			}
			valueSet_ = true;
		}
		else
		{
			this->skipCurrentElement();
		}
	}
	if(keySet_ && valueSet_)
	{
		if(entry->getAttachments()->hasKey(
			key_
		))
		{
			this->raiseError(
				"Duplicate attachment found"
			);
		}
		else
		{
			entry->getAttachments()->set(
				key_,
				value_
			);
		}
	}
	else
	{
		this->raiseError(
			"Entry binary key or value missing"
		);
	}
	return poolRef_;
}

QList<Entry*> KeePass2XmlReader::parseEntryHistory()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() ==
		"History"))
	{
		qWarning() << "Failed parsing History";
		return QList<Entry*>();
	}
	QList<Entry*> historyItems_;
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "Entry")
		{
			historyItems_.append(
				this->parseEntry(
					true
				)
			);
		}
		else
		{
			this->skipCurrentElement();
		}
	}
	return historyItems_;
}

TimeInfo KeePass2XmlReader::parseTimes()
{
	if(!(this->xml.isStartElement() && this->xml.name().toString() == "Times"))
	{
		qWarning() << "Failed parsing Times";
		return TimeInfo();
	}
	TimeInfo timeInfo_;
	while(!this->xml.error() && this->xml.readNextStartElement())
	{
		if(this->xml.name().toString() == "LastModificationTime")
		{
			timeInfo_.setLastModificationTime(
				this->readDateTime()
			);
		}
		else if(this->xml.name().toString() == "CreationTime")
		{
			timeInfo_.setCreationTime(
				this->readDateTime()
			);
		}
		else if(this->xml.name().toString() == "LastAccessTime")
		{
			timeInfo_.setLastAccessTime(
				this->readDateTime()
			);
		}
		else if(this->xml.name().toString() == "ExpiryTime")
		{
			timeInfo_.setExpiryTime(
				this->readDateTime()
			);
		}
		else if(this->xml.name().toString() == "Expires")
		{
			timeInfo_.setExpires(
				this->readBool()
			);
		}
		else if(this->xml.name().toString() == "UsageCount")
		{
			timeInfo_.setUsageCount(
				this->readNumber()
			);
		}
		else if(this->xml.name().toString() == "LocationChanged")
		{
			timeInfo_.setLocationChanged(
				this->readDateTime()
			);
		}
		else
		{
			this->skipCurrentElement();
		}
	}
	return timeInfo_;
}

QString KeePass2XmlReader::readString()
{
	return this->xml.readElementText();
	/*QString result_;
	if(this->xml.tokenType() == QXmlStreamReader::StartElement)
	{
		// Case-insensitive check
		// Elements where invalid characters might be present
		if(const QString currentElement_ = this->xml.name().toString().toLower()
			;
			currentElement_ == "value" || currentElement_ == "name" ||
			currentElement_ == "notes")
		{
			// Read the entire element content as plain text
			result_ = this->xml.readElementText(
				QXmlStreamReader::IncludeChildElements
			);
		}
		else
		{
			// For other elements, use the standard reading method
			result_ = this->xml.readElementText();
		}
	}
	else
	{
		// If not a start element, read the text content
		result_ = this->xml.text().toString();
	}
	return result_;*/
}

bool KeePass2XmlReader::readBool()
{
	if(const QString str_ = this->readString();
		str_.compare(
			"True",
			Qt::CaseInsensitive
		) == 0)
	{
		return true;
	}
	else if(str_.compare(
		"False",
		Qt::CaseInsensitive
	) == 0)
	{
		return false;
	}
	this->raiseError(
		"Invalid bool value"
	);
	return false;
}

QDateTime KeePass2XmlReader::readDateTime()
{
	const QString str_ = this->readString();
	QDateTime dt_ = QDateTime::fromString(
		str_,
		Qt::ISODate
	);
	if(!dt_.isValid())
	{
		if(this->strictMode)
		{
			this->raiseError(
				"Invalid date time value"
			);
		}
		else
		{
			dt_ = QDateTime::currentDateTimeUtc();
		}
	}
	return dt_;
}

QColor KeePass2XmlReader::readColor()
{
	QString colorStr_ = this->readString();
	if(colorStr_.isEmpty())
	{
		return QColor();
	}
	if(colorStr_.length() != 7 || colorStr_[0] != '#')
	{
		if(this->strictMode)
		{
			this->raiseError(
				"Invalid color value"
			);
		}
		return QColor();
	}
	QColor color_;
	for(auto i_ = 0; i_ <= 2; i_++)
	{
		QString rgbPartStr_ = colorStr_.mid(
			1 + 2 * i_,
			2
		);
		bool ok_;
		const int rgbPart_ = rgbPartStr_.toInt(
			&ok_,
			16
		);
		if(!ok_ || rgbPart_ > 255)
		{
			if(this->strictMode)
			{
				this->raiseError(
					"Invalid color rgb part"
				);
			}
			return QColor();
		}
		if(i_ == 0)
		{
			color_.setRed(
				rgbPart_
			);
		}
		else if(i_ == 1)
		{
			color_.setGreen(
				rgbPart_
			);
		}
		else
		{
			color_.setBlue(
				rgbPart_
			);
		}
	}
	return color_;
}

int KeePass2XmlReader::readNumber()
{
	bool ok_;
	const int result_ = this->readString().toInt(
		&ok_
	);
	if(!ok_)
	{
		this->raiseError(
			"Invalid number value"
		);
	}
	return result_;
}

UUID KeePass2XmlReader::readUUID()
{
	const QByteArray uuidBin_ = this->readBinary();
	if(uuidBin_.isEmpty())
	{
		return UUID();
	}
	if(uuidBin_.length() != UUID::Length)
	{
		if(this->strictMode)
		{
			this->raiseError(
				"Invalid uuid value"
			);
		}
		return UUID();
	}
	return UUID(
		uuidBin_
	);
}

QByteArray KeePass2XmlReader::readBinary()
{
	return QByteArray::fromBase64(
		this->readString().toLatin1()
	);
}

QByteArray KeePass2XmlReader::readCompressedBinary()
{
	QByteArray rawData_ = this->readBinary();
	QBuffer buffer_(
		&rawData_
	);
	buffer_.open(
		QIODevice::ReadOnly
	);
	QtIOCompressor compressor_(
		&buffer_
	);
	compressor_.setStreamFormat(
		QtIOCompressor::GzipFormat
	);
	compressor_.open(
		QIODevice::ReadOnly
	);
	QByteArray result_;
	if(!Tools::readAllFromDevice(
		&compressor_,
		result_
	))
	{
		this->raiseError(
			"Unable to decompress binary"
		);
	}
	return result_;
}

Group* KeePass2XmlReader::getGroup(
	const UUID &uuid
)
{
	if(this->groups.contains(
		uuid
	))
	{
		return this->groups.value(
			uuid
		);
	}
	const auto group_ = new Group();
	group_->setUpdateTimeinfo(
		false
	);
	group_->setUuid(
		uuid
	);
	group_->setParent(
		tmpParent
	);
	this->groups.insert(
		uuid,
		group_
	);
	return group_;
}

Entry* KeePass2XmlReader::getEntry(
	const UUID &uuid
)
{
	if(uuid.isNull())
	{
		qWarning() << "Entry is null";
		return nullptr;
	}
	if(this->entries.contains(
		uuid
	))
	{
		return this->entries.value(
			uuid
		);
	}
	const auto entry_ = new Entry();
	entry_->setUpdateTimeinfo(
		false
	);
	entry_->setUUID(
		uuid
	);
	entry_->setGroup(
		tmpParent
	);
	this->entries.insert(
		uuid,
		entry_
	);
	return entry_;
}

void KeePass2XmlReader::skipCurrentElement()
{
	qWarning(
		"KeePass2XmlReader::skipCurrentElement: skip element \"%s\"",
		qPrintable(
			this->xml.name().toString()
		)
	);
	this->xml.skipCurrentElement();
}
