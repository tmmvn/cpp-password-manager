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
#ifndef KEEPASSX_DATABASE_H
#define KEEPASSX_DATABASE_H
#include <QDateTime>
#include <QHash>
#include <QObject>
#include "core/UUID.h"
#include "keys/CompositeKey.h"
class Entry;
class Group;
class Metadata;
class QTimer;

struct DeletedObject
{
	UUID uuid;
	QDateTime deletionTime;
};

Q_DECLARE_TYPEINFO(
	DeletedObject,
	Q_MOVABLE_TYPE
);

class Database final:public QObject
{
	Q_OBJECT public:
	enum CompressionAlgorithm: u_int8_t
	{
		CompressionNone = 0,
		CompressionGZip = 1
	};

	static constexpr quint32 CompressionAlgorithmMax = CompressionGZip;

	struct DatabaseData
	{
		UUID cipher;
		CompressionAlgorithm compressionAlgo;
		QByteArray transformSeed;
		quint64 transformRounds;
		QByteArray transformedMasterKey;
		CompositeKey key;
		bool hasKey;
	};

	Database();
	virtual ~Database() override;
	Group* getRootGroup();
	const Group* getRootGroup() const;
	/**
	* Sets group as the root group and takes ownership of it.
	* Warning: Be careful when calling this method as it doesn't
	*          emit any notifications so e.g. models aren't updated.
	*          The caller is responsible for cleaning up the previous
				root group.
	*/
	void setRootGroup(
		Group* group
	);
	Metadata* getMetadata();
	const Metadata* getMetadata() const;
	Entry* resolveEntry(
		const UUID &uuid
	);
	Group* resolveGroup(
		const UUID &uuid
	);
	QList<DeletedObject> getDeletedObjects();
	void addDeletedObject(
		const DeletedObject &delObj
	);
	void addDeletedObject(
		const UUID &uuid
	);
	UUID getCipher() const;
	CompressionAlgorithm getCompressionAlgo() const;
	QByteArray transformSeed() const;
	quint64 transformRounds() const;
	QByteArray transformedMasterKey() const;
	void setCipher(
		const UUID &cipher
	);
	void setCompressionAlgo(
		CompressionAlgorithm algo
	);
	bool setTransformRounds(
		quint64 rounds
	);
	bool setKey(
		const CompositeKey &key,
		const QByteArray &transformSeed,
		bool updateChangedTime = true
	);
	/**
	* Sets the database key and generates a random transform seed.
	*/
	bool setKey(
		const CompositeKey &key
	);
	bool hasKey() const;
	bool verifyKey(
		const CompositeKey &key
	) const;
	void recycleEntry(
		Entry* entry
	);
	void recycleGroup(
		Group* group
	);
	void setEmitModified(
		bool value
	);
	void copyAttributesFrom(
		const Database* other
	);
	/**
	* Returns a unique id that is only valid as long as the Database exists.
	*/
	UUID getUUID();
	static Database* databaseByUUID(
		const UUID &uuid
	);
Q_SIGNALS:
	void sig_groupDataChanged(
		Group* group
	);
	void sig_groupAboutToAdd(
		Group* group,
		int index
	);
	void sig_groupAdded();
	void sig_groupAboutToRemove(
		Group* group
	);
	void sig_groupRemoved();
	void sig_groupAboutToMove(
		Group* group,
		Group* toGroup,
		int index
	);
	void sig_groupMoved();
	void sig_nameTextChanged();
	void sig_modified();
	void sig_modifiedImmediate();
private Q_SLOTS:
	void do_startModifiedTimer() const;
private:
	Entry* recFindEntry(
		const UUID &uuid,
		Group* group
	);
	Group* recFindGroup(
		const UUID &uuid,
		Group* group
	);
	void createRecycleBin();
	Metadata* const metadata;
	Group* rootGroup;
	QList<DeletedObject> deletedObjects;
	QTimer* timer;
	DatabaseData data;
	bool emitModified;
	UUID uuid;
	static QHash<UUID, Database*> uuidMap;
};
#endif // KEEPASSX_DATABASE_H
