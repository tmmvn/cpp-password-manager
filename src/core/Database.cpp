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
#include "Database.h"
#include <QFile>
#include <QTimer>
#include <QXmlStreamReader>
#include "core/Group.h"
#include "core/Metadata.h"
#include "crypto/Random.h"
#include "format/KeePass2.h"
QHash<UUID, Database*> Database::uuidMap;

Database::Database()
	: metadata(
		new Metadata(
			this
		)
	),
	timer(
		new QTimer(
			this
		)
	),
	emitModified(
		false
	),
	uuid(
		UUID::random()
	)
{
	this->data.cipher = KeePass2::CIPHER_AES;
	this->data.compressionAlgo = CompressionGZip;
	this->data.transformRounds = 100000;
	this->data.hasKey = false;
	this->setRootGroup(
		new Group()
	);
	this->getRootGroup()->setUuid(
		UUID::random()
	);
	this->timer->setSingleShot(
		true
	);
	this->uuidMap.insert(
		this->uuid,
		this
	);
	this->connect(
		this->metadata,
		&Metadata::sig_modified,
		this,
		&Database::sig_modifiedImmediate
	);
	this->connect(
		this->metadata,
		&Metadata::sig_nameTextChanged,
		this,
		&Database::sig_nameTextChanged
	);
	this->connect(
		this,
		&Database::sig_modifiedImmediate,
		this,
		&Database::do_startModifiedTimer
	);
	this->connect(
		this->timer,
		&QTimer::timeout,
		this,
		&Database::sig_modified
	);
}

Database::~Database()
{
	this->uuidMap.remove(
		this->uuid
	);
}

Group* Database::getRootGroup()
{
	return this->rootGroup;
}

const Group* Database::getRootGroup() const
{
	return this->rootGroup;
}

void Database::setRootGroup(
	Group* group
)
{
	if(group == nullptr)
	{
		return;
	}
	this->rootGroup = group;
	this->rootGroup->setParent(
		this
	);
}

Metadata* Database::getMetadata()
{
	return this->metadata;
}

const Metadata* Database::getMetadata() const
{
	return this->metadata;
}

Entry* Database::resolveEntry(
	const UUID &uuid
)
{
	return this->recFindEntry(
		uuid,
		this->rootGroup
	);
}

Entry* Database::recFindEntry(
	const UUID &uuid,
	Group* group
)
{
	const QList<Entry*> entryList_ = group->getEntries();
	for(Entry* entry_: entryList_)
	{
		if(entry_->getUUID() == uuid)
		{
			return entry_;
		}
	}
	const QList<Group*> children_ = group->getChildren();
	for(Group* child_: children_)
	{
		if(Entry* result_ = this->recFindEntry(
			uuid,
			child_
		))
		{
			return result_;
		}
	}
	return nullptr;
}

Group* Database::resolveGroup(
	const UUID &uuid
)
{
	return this->recFindGroup(
		uuid,
		this->rootGroup
	);
}

Group* Database::recFindGroup(
	const UUID &uuid,
	Group* group
)
{
	if(group->getUUID() == uuid)
	{
		return group;
	}
	const QList<Group*> children_ = group->getChildren();
	for(Group* child_: children_)
	{
		if(Group* result_ = this->recFindGroup(
			uuid,
			child_
		))
		{
			return result_;
		}
	}
	return nullptr;
}

QList<DeletedObject> Database::getDeletedObjects()
{
	return this->deletedObjects;
}

void Database::addDeletedObject(
	const DeletedObject &delObj
)
{
	if(delObj.deletionTime.timeSpec() != Qt::UTC)
	{
		return;
	}
	this->deletedObjects.append(
		delObj
	);
}

void Database::addDeletedObject(
	const UUID &uuid
)
{
	DeletedObject delObj_;
	delObj_.deletionTime = QDateTime::currentDateTimeUtc();
	delObj_.uuid = uuid;
	this->addDeletedObject(
		delObj_
	);
}

UUID Database::getCipher() const
{
	return this->data.cipher;
}

Database::CompressionAlgorithm Database::getCompressionAlgo() const
{
	return this->data.compressionAlgo;
}

QByteArray Database::transformSeed() const
{
	return this->data.transformSeed;
}

quint64 Database::transformRounds() const
{
	return this->data.transformRounds;
}

QByteArray Database::transformedMasterKey() const
{
	return this->data.transformedMasterKey;
}

void Database::setCipher(
	const UUID &cipher
)
{
	if(cipher.isNull())
	{
		return;
	}
	this->data.cipher = cipher;
}

void Database::setCompressionAlgo(
	const CompressionAlgorithm algo
)
{
	if(static_cast<quint32>(algo) > CompressionAlgorithmMax)
	{
		return;
	}
	this->data.compressionAlgo = algo;
}

bool Database::setTransformRounds(
	const quint64 rounds
)
{
	if(this->data.transformRounds != rounds)
	{
		const quint64 oldRounds_ = this->data.transformRounds;
		this->data.transformRounds = rounds;
		if(this->data.hasKey)
		{
			if(!this->setKey(
				this->data.key
			))
			{
				this->data.transformRounds = oldRounds_;
				return false;
			}
		}
	}
	return true;
}

bool Database::setKey(
	const CompositeKey &key,
	const QByteArray &transformSeed,
	const bool updateChangedTime
)
{
	bool ok_;
	QString errorString_;
	const QByteArray transformedMasterKey_ = key.transform(
		transformSeed,
		this->transformRounds(),
		&ok_,
		&errorString_
	);
	if(!ok_)
	{
		return false;
	}
	this->data.key = key;
	this->data.transformSeed = transformSeed;
	this->data.transformedMasterKey = transformedMasterKey_;
	this->data.hasKey = true;
	if(updateChangedTime)
	{
		this->metadata->setMasterKeyChanged(
			QDateTime::currentDateTimeUtc()
		);
	}
	sig_modifiedImmediate();
	return true;
}

bool Database::setKey(
	const CompositeKey &key
)
{
	return this->setKey(
		key,
		Random::getInstance()->getRandomArray(
			32
		)
	);
}

bool Database::hasKey() const
{
	return this->data.hasKey;
}

bool Database::verifyKey(
	const CompositeKey &key
) const
{
	if(!this->hasKey())
	{
		return false;
	}
	return (this->data.key.rawKey() == key.rawKey());
}

void Database::createRecycleBin()
{
	Group* recycleBin_ = Group::createRecycleBin();
	recycleBin_->setParent(
		this->getRootGroup()
	);
	this->metadata->setRecycleBin(
		recycleBin_
	);
}

void Database::recycleEntry(
	Entry* entry
)
{
	if(this->metadata->recycleBinEnabled())
	{
		if(!this->metadata->getRecycleBin())
		{
			this->createRecycleBin();
		}
		entry->setGroup(
			this->getMetadata()->getRecycleBin()
		);
	}
	else
	{
		delete entry;
	}
}

void Database::recycleGroup(
	Group* group
)
{
	if(this->metadata->recycleBinEnabled())
	{
		if(!this->metadata->getRecycleBin())
		{
			this->createRecycleBin();
		}
		group->setParent(
			this->getMetadata()->getRecycleBin()
		);
	}
	else
	{
		delete group;
	}
}

void Database::setEmitModified(
	const bool value
)
{
	if(this->emitModified && !value)
	{
		this->timer->stop();
	}
	this->emitModified = value;
}

void Database::copyAttributesFrom(
	const Database* other
)
{
	this->data = other->data;
	this->metadata->copyAttributesFrom(
		other->metadata
	);
}

UUID Database::getUUID()
{
	return uuid;
}

Database* Database::databaseByUUID(
	const UUID &uuid
)
{
	return uuidMap.value(
		uuid,
		nullptr
	);
}

void Database::do_startModifiedTimer() const
{
	if(!this->emitModified)
	{
		return;
	}
	if(this->timer->isActive())
	{
		this->timer->stop();
	}
	this->timer->start(
		150
	);
}
