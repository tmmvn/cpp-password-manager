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
#ifndef KEEPASSX_KEEPASS2XMLREADER_H
#define KEEPASSX_KEEPASS2XMLREADER_H
#include <QColor>
#include <QCoreApplication>
#include <QDateTime>
#include <QHash>
#include <QPair>
#include <QXmlStreamReader>
#include "core/TimeInfo.h"
#include "core/UUID.h"
class QBuffer;
class Database;
class Entry;
class Group;
class KeePass2RandomStream;
class Metadata;

class KeePass2XmlReader
{
	Q_DECLARE_TR_FUNCTIONS(
		KeePass2XmlReader
	)
public:
	KeePass2XmlReader();
	Database* readDatabase(
		QIODevice* device
	);
	void readDatabase(
		QIODevice* device,
		Database* db,
		KeePass2RandomStream* randomStream = nullptr
	);
	Database* readDatabase(
		const QString &filename
	);
	bool hasError() const;
	QString getErrorString();
	QByteArray getHeaderHash();
	void setStrictMode(
		bool strictMode
	);
private:
	bool parseKeePassFile();
	void parseMeta();
	void parseMemoryProtection();
	void parseCustomIcons();
	void parseIcon();
	void parseBinaries();
	void parseCustomData();
	void parseCustomDataItem();
	bool parseRoot();
	Group* parseGroup();
	void parseDeletedObjects();
	void parseDeletedObject();
	Entry* parseEntry(
		bool history
	);
	void parseEntryString(
		Entry* entry
	);
	QPair<QString, QString> parseEntryBinary(
		Entry* entry
	);
	QList<Entry*> parseEntryHistory();
	TimeInfo parseTimes();
	QString readString();
	bool readBool();
	QDateTime readDateTime();
	QColor readColor();
	int readNumber();
	UUID readUUID();
	QByteArray readBinary();
	QByteArray readCompressedBinary();
	Group* getGroup(
		const UUID &uuid
	);
	Entry* getEntry(
		const UUID &uuid
	);
	void raiseError(
		const QString &errorMessage
	);
	void skipCurrentElement();
	QXmlStreamReader xml;
	KeePass2RandomStream* randomStream;
	Database* db;
	Metadata* meta;
	Group* tmpParent;
	QHash<UUID, Group*> groups;
	QHash<UUID, Entry*> entries;
	QHash<QString, QByteArray> binaryPool;
	QMultiHash<QString, QPair<Entry*, QString>> binaryMap;
	QByteArray headerHash;
	bool error;
	QString errorStr;
	bool strictMode;
};
#endif // KEEPASSX_KEEPASS2XMLREADER_H
