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
#ifndef KEEPASSX_KEEPASS2XMLWRITER_H
#define KEEPASSX_KEEPASS2XMLWRITER_H
#include <QColor>
#include <QXmlStreamWriter>
#include "core/Database.h"
#include "core/Entry.h"
#include "core/Group.h"
#include "core/TimeInfo.h"
#include "core/UUID.h"
class KeePass2RandomStream;
class Metadata;

class KeePass2XmlWriter
{
public:
	KeePass2XmlWriter();
	void writeDatabase(
		QIODevice* device,
		Database* db,
		KeePass2RandomStream* randomStream = nullptr,
		const QByteArray &headerHash = QByteArray()
	);
	void writeDatabase(
		const QString &filename,
		Database* db
	);
	bool hasError() const;
	QString getErrorString();
private:
	void generateIdMap();
	void writeMetadata();
	void writeMemoryProtection();
	void writeCustomIcons();
	void writeIcon(
		const UUID &uuid,
		const QImage &icon
	);
	void writeBinaries();
	void writeCustomData();
	void writeCustomDataItem(
		const QString &key,
		const QString &value
	);
	void writeRoot();
	void writeGroup(
		const Group* group
	);
	void writeTimes(
		const TimeInfo &ti
	);
	void writeDeletedObjects();
	void writeDeletedObject(
		const DeletedObject &delObj
	);
	void writeEntry(
		const Entry* entry
	);
	void writeEntryHistory(
		const Entry* entry
	);
	void writeString(
		const QString &qualifiedName,
		const QString &string
	);
	void writeNumber(
		const QString &qualifiedName,
		int number
	);
	void writeBool(
		const QString &qualifiedName,
		bool b
	);
	void writeDateTime(
		const QString &qualifiedName,
		const QDateTime &dateTime
	);
	void writeUUID(
		const QString &qualifiedName,
		const UUID &uuid
	);
	void writeUUID(
		const QString &qualifiedName,
		const Group* group
	);
	void writeUUID(
		const QString &qualifiedName,
		const Entry* entry
	);
	void writeBinary(
		const QString &qualifiedName,
		const QByteArray &ba
	);
	void writeColor(
		const QString &qualifiedName,
		const QColor &color
	);
	void writeTriState(
		const QString &qualifiedName,
		Group::TriState triState
	);
	static QString colorPartToString(
		int value
	);
	static QString stripInvalidXml10Chars(
		const QString &str
	);
	void raiseError(
		const QString &errorMessage
	);
	QXmlStreamWriter xml;
	Database* db;
	Metadata* meta;
	KeePass2RandomStream* randomStream;
	QByteArray headerHash;
	QHash<QByteArray, int> idMap;
	bool error;
	QString errorStr;
};
#endif // KEEPASSX_KEEPASS2XMLWRITER_H
