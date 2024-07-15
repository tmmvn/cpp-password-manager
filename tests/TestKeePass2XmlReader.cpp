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
#include "TestKeePass2XmlReader.h"
#include <QBuffer>
#include <QFile>
#include <QTest>
#include "core/Database.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "crypto/Crypto.h"
#include "format/KeePass2XmlReader.h"
#include "format/KeePass2XmlWriter.h"
#include "config-keepassx-tests.h"
QTEST_GUILESS_MAIN(
	TestKeePass2XmlReader
)

namespace QTest
{
	template<> char* toString(
		const UUID &uuid
	)
	{
		QByteArray ba = "Uuid(";
		ba += uuid.toBase64().toLatin1().constData();
		ba += ")";
		return qstrdup(
			ba.constData()
		);
	}

	template<> char* toString(
		const Group::TriState &triState
	)
	{
		QString value;
		if(triState == Group::Inherit)
		{
			value = "null";
		}
		else if(triState == Group::Enable)
		{
			value = "true";
		}
		else
		{
			value = "false";
		}
		return qstrdup(
			value.toLocal8Bit().constData()
		);
	}
}

QDateTime TestKeePass2XmlReader::genDT(
	int year,
	int month,
	int day,
	int hour,
	int min,
	int second
)
{
	QDate date(
		year,
		month,
		day
	);
	QTime time(
		hour,
		min,
		second
	);
	return QDateTime(
		date,
		time,
		Qt::UTC
	);
}

QByteArray TestKeePass2XmlReader::strToBytes(
	const QString &str
)
{
	QByteArray result;
	for(int i = 0; i < str.size(); i++)
	{
		result.append(
			str.at(
				i
			).unicode() >> 8
		);
		result.append(
			str.at(
				i
			).unicode() & 0xFF
		);
	}
	return result;
}

void TestKeePass2XmlReader::initTestCase()
{
	QVERIFY(
		Crypto::init()
	);
	KeePass2XmlReader reader;
	reader.setStrictMode(
		true
	);
	QString xmlFile = QString(
		KEEPASSX_TEST_DATA_DIR
	).append(
		"/NewDatabase.xml"
	);
	m_db = reader.readDatabase(
		xmlFile
	);
	QVERIFY(
		m_db
	);
	QVERIFY(
		!reader.hasError()
	);
}

void TestKeePass2XmlReader::testMetadata()
{
	QCOMPARE(
		m_db->getMetadata()->getGenerator(),
		QString("KeePass")
	);
	QCOMPARE(
		m_db->getMetadata()->getName(),
		QString("ANAME")
	);
	QCOMPARE(
		m_db->getMetadata()->getNameChangedTime(),
		genDT(2010, 8, 8, 17, 24, 53)
	);
	QCOMPARE(
		m_db->getMetadata()->getDescription(),
		QString("ADESC")
	);
	QCOMPARE(
		m_db->getMetadata()->getDescriptionTimeChanged(),
		genDT(2010, 8, 8, 17, 27, 12)
	);
	QCOMPARE(
		m_db->getMetadata()->getDefaultUserName(),
		QString("DEFUSERNAME")
	);
	QCOMPARE(
		m_db->getMetadata()->getDefaultUserNameChanged(),
		genDT(2010, 8, 8, 17, 27, 45)
	);
	QCOMPARE(
		m_db->getMetadata()->getMaintenanceHistoryDays(),
		127
	);
	QCOMPARE(
		m_db->getMetadata()->getColor(),
		QColor(0xff, 0xef, 0x00)
	);
	QCOMPARE(
		m_db->getMetadata()->getMasterKeyChanged(),
		genDT(2012, 4, 5, 17, 9, 34)
	);
	QCOMPARE(
		m_db->getMetadata()->masterKeyChangeRec(),
		101
	);
	QCOMPARE(
		m_db->getMetadata()->masterKeyChangeForce(),
		-1
	);
	QCOMPARE(
		m_db->getMetadata()->protectTitle(),
		false
	);
	QCOMPARE(
		m_db->getMetadata()->protectUsername(),
		true
	);
	QCOMPARE(
		m_db->getMetadata()->protectPassword(),
		false
	);
	QCOMPARE(
		m_db->getMetadata()->protectUrl(),
		true
	);
	QCOMPARE(
		m_db->getMetadata()->protectNotes(),
		false
	);
	QCOMPARE(
		m_db->getMetadata()->recycleBinEnabled(),
		true
	);
	QVERIFY(
		m_db->getMetadata()->getRecycleBin() != nullptr
	);
	QCOMPARE(
		m_db->getMetadata()->getRecycleBin()->getName(),
		QString("Recycle Bin")
	);
	QCOMPARE(
		m_db->getMetadata()->getRecycleBinChangedTime(),
		genDT(2010, 8, 25, 16, 12, 57)
	);
	QVERIFY(
		m_db->getMetadata()->getEntryTemplatesGroup() == nullptr
	);
	QCOMPARE(
		m_db->getMetadata()->getEntryTemplatesGroupChangedTime(),
		genDT(2010, 8, 8, 17, 24, 19)
	);
	QVERIFY(
		m_db->getMetadata()->getLastSelectedGroup() != nullptr
	);
	QCOMPARE(
		m_db->getMetadata()->getLastSelectedGroup()->getName(),
		QString("NewDatabase")
	);
	QVERIFY(
		m_db->getMetadata()->getLastTopVisibleGroup() == m_db->getMetadata()->
		getLastSelectedGroup()
	);
	QCOMPARE(
		m_db->getMetadata()->getHistoryMaxItems(),
		-1
	);
	QCOMPARE(
		m_db->getMetadata()->getHistoryMaxSize(),
		5242880
	);
}

void TestKeePass2XmlReader::testCustomIcons()
{
	QCOMPARE(
		m_db->getMetadata()->getCustomIcons().size(),
		1
	);
	UUID uuid = UUID::fromBase64(
		"++vyI+daLk6omox4a6kQGA=="
	);
	QVERIFY(
		m_db->getMetadata()->getCustomIcons().contains(uuid)
	);
	QImage icon = m_db->getMetadata()->getCustomIcon(
		uuid
	);
	QCOMPARE(
		icon.width(),
		16
	);
	QCOMPARE(
		icon.height(),
		16
	);
	for(int x = 0; x < 16; x++)
	{
		for(int y = 0; y < 16; y++)
		{
			QRgb rgb = icon.pixel(
				x,
				y
			);
			QCOMPARE(
				qRed(rgb),
				128
			);
			QCOMPARE(
				qGreen(rgb),
				0
			);
			QCOMPARE(
				qBlue(rgb),
				128
			);
		}
	}
}

void TestKeePass2XmlReader::testCustomData()
{
	QHash<QString, QString> customFields = m_db->getMetadata()->
		getCustomFields();
	QCOMPARE(
		customFields.size(),
		2
	);
	QCOMPARE(
		customFields.value("A Sample Test Key"),
		QString("valu")
	);
	QCOMPARE(
		customFields.value("custom key"),
		QString("blub")
	);
}

void TestKeePass2XmlReader::testGroupRoot()
{
	const Group* group = m_db->getRootGroup();
	QVERIFY(
		group
	);
	QCOMPARE(
		group->getUUID().toBase64(),
		QString("lmU+9n0aeESKZvcEze+bRg==")
	);
	QCOMPARE(
		group->getName(),
		QString("NewDatabase")
	);
	QCOMPARE(
		group->getNotes(),
		QString("")
	);
	QCOMPARE(
		group->getIconNumber(),
		49
	);
	QCOMPARE(
		group->getIconUUID(),
		UUID()
	);
	QVERIFY(
		group->isExpanded()
	);
	TimeInfo ti = group->getTimeInfo();
	QCOMPARE(
		ti.getLastModificationTime(),
		genDT(2010, 8, 8, 17, 24, 27)
	);
	QCOMPARE(
		ti.getCreationTime(),
		genDT(2010, 8, 7, 17, 24, 27)
	);
	QCOMPARE(
		ti.getLastAccessTime(),
		genDT(2010, 8, 9, 9, 9, 44)
	);
	QCOMPARE(
		ti.getExpiryTime(),
		genDT(2010, 8, 8, 17, 24, 17)
	);
	QVERIFY(
		!ti.getExpires()
	);
	QCOMPARE(
		ti.getUsageCount(),
		52
	);
	QCOMPARE(
		ti.getLocationChanged(),
		genDT(2010, 8, 8, 17, 24, 27)
	);
	QCOMPARE(
		group->isSearchingEnabled(),
		Group::Inherit
	);
	QCOMPARE(
		group->getLastTopVisibleEntry()->getUUID().toBase64(),
		QString("+wSUOv6qf0OzW8/ZHAs2sA==")
	);
	QCOMPARE(
		group->getChildren().size(),
		3
	);
	QVERIFY(
		m_db->getMetadata()->getRecycleBin() == m_db->getRootGroup() ->
		getChildren() .at(2)
	);
	QCOMPARE(
		group->getEntries().size(),
		2
	);
}

void TestKeePass2XmlReader::testGroup1()
{
	const Group* group = m_db->getRootGroup()->getChildren().at(
		0
	);
	QCOMPARE(
		group->getUUID().toBase64(),
		QString("AaUYVdXsI02h4T1RiAlgtg==")
	);
	QCOMPARE(
		group->getName(),
		QString("General")
	);
	QCOMPARE(
		group->getNotes(),
		QString("Group Notez")
	);
	QCOMPARE(
		group->getIconNumber(),
		48
	);
	QCOMPARE(
		group->getIconUUID(),
		UUID()
	);
	QCOMPARE(
		group->isExpanded(),
		true
	);
	QCOMPARE(
		group->isSearchingEnabled(),
		Group::Disable
	);
	QVERIFY(
		!group->getLastTopVisibleEntry()
	);
}

void TestKeePass2XmlReader::testGroup2()
{
	const Group* group = m_db->getRootGroup()->getChildren().at(
		1
	);
	QCOMPARE(
		group->getUUID().toBase64(),
		QString("1h4NtL5DK0yVyvaEnN//4A==")
	);
	QCOMPARE(
		group->getName(),
		QString("Windows")
	);
	QCOMPARE(
		group->isExpanded(),
		false
	);
	QCOMPARE(
		group->getChildren().size(),
		1
	);
	const Group* child = group->getChildren().first();
	QCOMPARE(
		child->getUUID().toBase64(),
		QString("HoYE/BjLfUSW257pCHJ/eA==")
	);
	QCOMPARE(
		child->getName(),
		QString("Subsub")
	);
	QCOMPARE(
		child->getEntries().size(),
		1
	);
	const Entry* entry = child->getEntries().first();
	QCOMPARE(
		entry->getUUID().toBase64(),
		QString("GZpdQvGXOU2kaKRL/IVAGg==")
	);
	QCOMPARE(
		entry->getTitle(),
		QString("Subsub Entry")
	);
}

void TestKeePass2XmlReader::testEntry1()
{
	const Entry* entry = m_db->getRootGroup()->getEntries().at(
		0
	);
	QCOMPARE(
		entry->getUUID().toBase64(),
		QString("+wSUOv6qf0OzW8/ZHAs2sA==")
	);
	QCOMPARE(
		entry->getHistoryItems().size(),
		2
	);
	QCOMPARE(
		entry->getIconNumber(),
		0
	);
	QCOMPARE(
		entry->getIconUUID(),
		UUID()
	);
	QVERIFY(
		!entry->getForegroundColor().isValid()
	);
	QVERIFY(
		!entry->getBackgroundColor().isValid()
	);
	QCOMPARE(
		entry->getOverrideURL(),
		QString("")
	);
	QCOMPARE(
		entry->getTags(),
		QString("a b c")
	);
	const TimeInfo ti = entry->getTimeInfo();
	QCOMPARE(
		ti.getLastModificationTime(),
		genDT(2010, 8, 25, 16, 19, 25)
	);
	QCOMPARE(
		ti.getCreationTime(),
		genDT(2010, 8, 25, 16, 13, 54)
	);
	QCOMPARE(
		ti.getLastAccessTime(),
		genDT(2010, 8, 25, 16, 19, 25)
	);
	QCOMPARE(
		ti.getExpiryTime(),
		genDT(2010, 8, 25, 16, 12, 57)
	);
	QVERIFY(
		!ti.getExpires()
	);
	QCOMPARE(
		ti.getUsageCount(),
		8
	);
	QCOMPARE(
		ti.getLocationChanged(),
		genDT(2010, 8, 25, 16, 13, 54)
	);
	QList<QString> attrs = entry->getAttributes()->getKeys();
	QCOMPARE(
		entry->getAttributes()->getValue("Notes"),
		QString("Notes")
	);
	QVERIFY(
		!entry->getAttributes()->isProtected("Notes")
	);
	QVERIFY(
		attrs.removeOne("Notes")
	);
	QCOMPARE(
		entry->getAttributes()->getValue("Password"),
		QString("Password")
	);
	QVERIFY(
		!entry->getAttributes()->isProtected("Password")
	);
	QVERIFY(
		attrs.removeOne("Password")
	);
	QCOMPARE(
		entry->getAttributes()->getValue("Title"),
		QString("Sample Entry 1")
	);
	QVERIFY(
		!entry->getAttributes()->isProtected("Title")
	);
	QVERIFY(
		attrs.removeOne("Title")
	);
	QCOMPARE(
		entry->getAttributes()->getValue("URL"),
		QString("")
	);
	QVERIFY(
		entry->getAttributes()->isProtected("URL")
	);
	QVERIFY(
		attrs.removeOne("URL")
	);
	QCOMPARE(
		entry->getAttributes()->getValue("UserName"),
		QString("User Name")
	);
	QVERIFY(
		entry->getAttributes()->isProtected("UserName")
	);
	QVERIFY(
		attrs.removeOne("UserName")
	);
	QVERIFY(
		attrs.isEmpty()
	);
	QCOMPARE(
		entry->getTitle(),
		entry->getAttributes()->getValue("Title")
	);
	QCOMPARE(
		entry->getURL(),
		entry->getAttributes()->getValue("URL")
	);
	QCOMPARE(
		entry->getUsername(),
		entry->getAttributes()->getValue("UserName")
	);
	QCOMPARE(
		entry->getPassword(),
		entry->getAttributes()->getValue("Password")
	);
	QCOMPARE(
		entry->getNotes(),
		entry->getAttributes()->getValue("Notes")
	);
	QCOMPARE(
		entry->getAttachments()->getKeys().size(),
		1
	);
	QCOMPARE(
		entry->getAttachments()->getValue("myattach.txt"),
		QByteArray("abcdefghijk")
	);
	QCOMPARE(
		entry->getHistoryItems().at(0)->getAttachments()->getKeys().size(),
		1
	);
	QCOMPARE(
		entry->getHistoryItems().at(0)->getAttachments()->getValue(
			"myattach.txt"),
		QByteArray("0123456789")
	);
	QCOMPARE(
		entry->getHistoryItems().at(1)->getAttachments()->getKeys().size(),
		1
	);
	QCOMPARE(
		entry->getHistoryItems().at(1)->getAttachments()->getValue(
			"myattach.txt"),
		QByteArray("abcdefghijk")
	);
}

void TestKeePass2XmlReader::testEntry2()
{
	const Entry* entry = m_db->getRootGroup()->getEntries().at(
		1
	);
	QCOMPARE(
		entry->getUUID().toBase64(),
		QString("4jbADG37hkiLh2O0qUdaOQ==")
	);
	QCOMPARE(
		entry->getIconNumber(),
		0
	);
	QCOMPARE(
		entry->getIconUUID().toBase64(),
		QString("++vyI+daLk6omox4a6kQGA==")
	);
	// TODO: test entry->icon()
	QCOMPARE(
		entry->getForegroundColor(),
		QColor(255, 0, 0)
	);
	QCOMPARE(
		entry->getBackgroundColor(),
		QColor(255, 255, 0)
	);
	QCOMPARE(
		entry->getOverrideURL(),
		QString("http://override.net/")
	);
	QCOMPARE(
		entry->getTags(),
		QString("")
	);
	const TimeInfo ti = entry->getTimeInfo();
	QCOMPARE(
		ti.getUsageCount(),
		7
	);
	QList<QString> attrs = entry->getAttributes()->getKeys();
	QCOMPARE(
		entry->getAttributes()->getValue("CustomString"),
		QString("isavalue")
	);
	QVERIFY(
		attrs.removeOne("CustomString")
	);
	QCOMPARE(
		entry->getAttributes()->getValue("Notes"),
		QString("")
	);
	QVERIFY(
		attrs.removeOne("Notes")
	);
	QCOMPARE(
		entry->getAttributes()->getValue("Password"),
		QString("Jer60Hz8o9XHvxBGcRqT")
	);
	QVERIFY(
		attrs.removeOne("Password")
	);
	QCOMPARE(
		entry->getAttributes()->getValue("Protected String"),
		QString("y")
	); // TODO: should have a protection attribute
	QVERIFY(
		attrs.removeOne("Protected String")
	);
	QCOMPARE(
		entry->getAttributes()->getValue("Title"),
		QString("Sample Entry 2")
	);
	QVERIFY(
		attrs.removeOne("Title")
	);
	QCOMPARE(
		entry->getAttributes()->getValue("URL"),
		QString("http://www.keepassx.org/")
	);
	QVERIFY(
		attrs.removeOne("URL")
	);
	QCOMPARE(
		entry->getAttributes()->getValue("UserName"),
		QString("notDEFUSERNAME")
	);
	QVERIFY(
		attrs.removeOne("UserName")
	);
	QVERIFY(
		attrs.isEmpty()
	);
	QCOMPARE(
		entry->getAttachments()->getKeys().size(),
		1
	);
	QCOMPARE(
		QString::fromLatin1(entry->getAttachments()->getValue("myattach.txt")),
		QString("abcdefghijk")
	);
}

void TestKeePass2XmlReader::testEntryHistory()
{
	const Entry* entryMain = m_db->getRootGroup()->getEntries().at(
		0
	);
	QCOMPARE(
		entryMain->getHistoryItems().size(),
		2
	); {
		const Entry* entry = entryMain->getHistoryItems().at(
			0
		);
		QCOMPARE(
			entry->getUUID(),
			entryMain->getUUID()
		);
		QVERIFY(
			!entry->parent()
		);
		QCOMPARE(
			entry->getTimeInfo().getLastModificationTime(),
			genDT(2010, 8, 25, 16, 13, 54)
		);
		QCOMPARE(
			entry->getTimeInfo().getUsageCount(),
			3
		);
		QCOMPARE(
			entry->getTitle(),
			QString("Sample Entry")
		);
		QCOMPARE(
			entry->getURL(),
			QString("http://www.somesite.com/")
		);
	} {
		const Entry* entry = entryMain->getHistoryItems().at(
			1
		);
		QCOMPARE(
			entry->getUUID(),
			entryMain->getUUID()
		);
		QVERIFY(
			!entry->parent()
		);
		QCOMPARE(
			entry->getTimeInfo().getLastModificationTime(),
			genDT(2010, 8, 25, 16, 15, 43)
		);
		QCOMPARE(
			entry->getTimeInfo().getUsageCount(),
			7
		);
		QCOMPARE(
			entry->getTitle(),
			QString("Sample Entry 1")
		);
		QCOMPARE(
			entry->getURL(),
			QString("http://www.somesite.com/")
		);
	}
}

void TestKeePass2XmlReader::testDeletedObjects()
{
	QList<DeletedObject> objList = m_db->getDeletedObjects();
	DeletedObject delObj;
	delObj = objList.takeFirst();
	QCOMPARE(
		delObj.uuid.toBase64(),
		QString("5K/bzWCSmkCv5OZxYl4N/w==")
	);
	QCOMPARE(
		delObj.deletionTime,
		genDT(2010, 8, 25, 16, 14, 12)
	);
	delObj = objList.takeFirst();
	QCOMPARE(
		delObj.uuid.toBase64(),
		QString("80h8uSNWgkKhKCp1TgXF7g==")
	);
	QCOMPARE(
		delObj.deletionTime,
		genDT(2010, 8, 25, 16, 14, 14)
	);
	QVERIFY(
		objList.isEmpty()
	);
}

void TestKeePass2XmlReader::testBroken()
{
	QFETCH(
		QString,
		baseName
	);
	QFETCH(
		bool,
		strictMode
	);
	QFETCH(
		bool,
		expectError
	);
	KeePass2XmlReader reader;
	reader.setStrictMode(
		strictMode
	);
	QString xmlFile = QString(
		"%1/%2.xml"
	).arg(
		KEEPASSX_TEST_DATA_DIR,
		baseName
	);
	QVERIFY(
		QFile::exists(xmlFile)
	);
	QScopedPointer<Database> db(
		reader.readDatabase(
			xmlFile
		)
	);
	if(reader.hasError())
	{
		qWarning(
			"Reader error: %s",
			qPrintable(
				reader.getErrorString()
			)
		);
	}
	QCOMPARE(
		reader.hasError(),
		expectError
	);
}

void TestKeePass2XmlReader::testBroken_data()
{
	QTest::addColumn<QString>(
		"baseName"
	);
	QTest::addColumn<bool>(
		"strictMode"
	);
	QTest::addColumn<bool>(
		"expectError"
	);
	//                                                    testfile                 strict?  error?
	QTest::newRow(
		"BrokenNoGroupUuid     (strict)"
	) << "BrokenNoGroupUuid" << true << true;
	QTest::newRow(
		"BrokenNoGroupUuid (not strict)"
	) << "BrokenNoGroupUuid" << false << false;
	QTest::newRow(
		"BrokenNoEntryUuid     (strict)"
	) << "BrokenNoEntryUuid" << true << true;
	QTest::newRow(
		"BrokenNoEntryUuid (not strict)"
	) << "BrokenNoEntryUuid" << false << false;
	QTest::newRow(
		"BrokenNoRootGroup     (strict)"
	) << "BrokenNoRootGroup" << true << true;
	QTest::newRow(
		"BrokenNoRootGroup (not strict)"
	) << "BrokenNoRootGroup" << false << true;
	QTest::newRow(
		"BrokenTwoRoots     (strict)"
	) << "BrokenTwoRoots" << true << true;
	QTest::newRow(
		"BrokenTwoRoots (not strict)"
	) << "BrokenTwoRoots" << false << true;
	QTest::newRow(
		"BrokenTwoRootGroups     (strict)"
	) << "BrokenTwoRootGroups" << true << true;
	QTest::newRow(
		"BrokenTwoRootGroups (not strict)"
	) << "BrokenTwoRootGroups" << false << true;
	QTest::newRow(
		"BrokenGroupReference     (strict)"
	) << "BrokenGroupReference" << true << false;
	QTest::newRow(
		"BrokenGroupReference (not strict)"
	) << "BrokenGroupReference" << false << false;
	QTest::newRow(
		"BrokenDeletedObjects     (strict)"
	) << "BrokenDeletedObjects" << true << true;
	QTest::newRow(
		"BrokenDeletedObjects (not strict)"
	) << "BrokenDeletedObjects" << false << false;
	QTest::newRow(
		"BrokenDifferentEntryHistoryUuid (strict)"
	) << "BrokenDifferentEntryHistoryUuid" << true << true;
	QTest::newRow(
		"BrokenDifferentEntryHistoryUuid (not strict)"
	) << "BrokenDifferentEntryHistoryUuid" << false << false;
}

void TestKeePass2XmlReader::testEmptyUuids()
{
	KeePass2XmlReader reader;
	reader.setStrictMode(
		true
	);
	QString xmlFile = QString(
		"%1/%2.xml"
	).arg(
		KEEPASSX_TEST_DATA_DIR,
		"EmptyUuids"
	);
	QVERIFY(
		QFile::exists(xmlFile)
	);
	QScopedPointer<Database> db(
		reader.readDatabase(
			xmlFile
		)
	);
	if(reader.hasError())
	{
		qWarning(
			"Reader error: %s",
			qPrintable(
				reader.getErrorString()
			)
		);
	}
	QVERIFY(
		!reader.hasError()
	);
}

void TestKeePass2XmlReader::testInvalidXmlChars()
{
	QScopedPointer<Database> dbWrite(
		new Database()
	);
	QString strPlainInvalid = QString().append(
		QChar(
			0x02
		)
	).append(
		QChar(
			0x19
		)
	).append(
		QChar(
			0xFFFE
		)
	).append(
		QChar(
			0xFFFF
		)
	);
	QString strPlainValid = QString().append(
		QChar(
			0x09
		)
	).append(
		QChar(
			0x0A
		)
	).append(
		QChar(
			0x20
		)
	).append(
		QChar(
			0xD7FF
		)
	).append(
		QChar(
			0xE000
		)
	).append(
		QChar(
			0xFFFD
		)
	);
	// U+10437 in UTF-16: D801 DC37
	//                    high low  surrogate
	QString strSingleHighSurrogate1 = QString().append(
		QChar(
			0xD801
		)
	);
	QString strSingleHighSurrogate2 = QString().append(
		QChar(
			0x31
		)
	).append(
		QChar(
			0xD801
		)
	).append(
		QChar(
			0x32
		)
	);
	QString strHighHighSurrogate = QString().append(
		QChar(
			0xD801
		)
	).append(
		QChar(
			0xD801
		)
	);
	QString strSingleLowSurrogate1 = QString().append(
		QChar(
			0xDC37
		)
	);
	QString strSingleLowSurrogate2 = QString().append(
		QChar(
			(0x31)
		)
	).append(
		QChar(
			0xDC37
		)
	).append(
		QChar(
			0x32
		)
	);
	QString strLowLowSurrogate = QString().append(
		QChar(
			0xDC37
		)
	).append(
		QChar(
			0xDC37
		)
	);
	QString strSurrogateValid1 = QString().append(
		QChar(
			0xD801
		)
	).append(
		QChar(
			0xDC37
		)
	);
	QString strSurrogateValid2 = QString().append(
		QChar(
			0x31
		)
	).append(
		QChar(
			0xD801
		)
	).append(
		QChar(
			0xDC37
		)
	).append(
		QChar(
			0x32
		)
	);
	Entry* entry = new Entry();
	entry->setUUID(
		UUID::random()
	);
	entry->setGroup(
		dbWrite->getRootGroup()
	);
	entry->getAttributes()->set(
		"PlainInvalid",
		strPlainInvalid
	);
	entry->getAttributes()->set(
		"PlainValid",
		strPlainValid
	);
	entry->getAttributes()->set(
		"SingleHighSurrogate1",
		strSingleHighSurrogate1
	);
	entry->getAttributes()->set(
		"SingleHighSurrogate2",
		strSingleHighSurrogate2
	);
	entry->getAttributes()->set(
		"HighHighSurrogate",
		strHighHighSurrogate
	);
	entry->getAttributes()->set(
		"SingleLowSurrogate1",
		strSingleLowSurrogate1
	);
	entry->getAttributes()->set(
		"SingleLowSurrogate2",
		strSingleLowSurrogate2
	);
	entry->getAttributes()->set(
		"LowLowSurrogate",
		strLowLowSurrogate
	);
	entry->getAttributes()->set(
		"SurrogateValid1",
		strSurrogateValid1
	);
	entry->getAttributes()->set(
		"SurrogateValid2",
		strSurrogateValid2
	);
	QBuffer buffer;
	buffer.open(
		QIODevice::ReadWrite
	);
	KeePass2XmlWriter writer;
	writer.writeDatabase(
		&buffer,
		dbWrite.data()
	);
	QVERIFY(
		!writer.hasError()
	);
	buffer.seek(
		0
	);
	KeePass2XmlReader reader;
	reader.setStrictMode(
		true
	);
	QScopedPointer<Database> dbRead(
		reader.readDatabase(
			&buffer
		)
	);
	if(reader.hasError())
	{
		qWarning(
			"Database read error: %s",
			qPrintable(
				reader.getErrorString()
			)
		);
	}
	QVERIFY(
		!reader.hasError()
	);
	QVERIFY(
		!dbRead.isNull()
	);
	QCOMPARE(
		dbRead->getRootGroup()->getEntries().size(),
		1
	);
	Entry* entryRead = dbRead->getRootGroup()->getEntries().at(
		0
	);
	EntryAttributes* attrRead = entryRead->getAttributes();
	QCOMPARE(
		strToBytes(attrRead->getValue("PlainInvalid")),
		QByteArray()
	);
	QCOMPARE(
		strToBytes(attrRead->getValue("PlainValid")),
		strToBytes(strPlainValid)
	);
	QCOMPARE(
		strToBytes(attrRead->getValue("SingleHighSurrogate1")),
		QByteArray()
	);
	QCOMPARE(
		strToBytes(attrRead->getValue("SingleHighSurrogate2")),
		strToBytes(QString("12"))
	);
	QCOMPARE(
		strToBytes(attrRead->getValue("HighHighSurrogate")),
		QByteArray()
	);
	QCOMPARE(
		strToBytes(attrRead->getValue("SingleLowSurrogate1")),
		QByteArray()
	);
	QCOMPARE(
		strToBytes(attrRead->getValue("SingleLowSurrogate2")),
		strToBytes(QString("12"))
	);
	QCOMPARE(
		strToBytes(attrRead->getValue("LowLowSurrogate")),
		QByteArray()
	);
	QCOMPARE(
		strToBytes(attrRead->getValue("SurrogateValid1")),
		strToBytes(strSurrogateValid1)
	);
	QCOMPARE(
		strToBytes(attrRead->getValue("SurrogateValid2")),
		strToBytes(strSurrogateValid2)
	);
}

void TestKeePass2XmlReader::testRepairUuidHistoryItem()
{
	KeePass2XmlReader reader;
	QString xmlFile = QString(
		"%1/%2.xml"
	).arg(
		KEEPASSX_TEST_DATA_DIR,
		"BrokenDifferentEntryHistoryUuid"
	);
	QVERIFY(
		QFile::exists(xmlFile)
	);
	QScopedPointer<Database> db(
		reader.readDatabase(
			xmlFile
		)
	);
	if(reader.hasError())
	{
		qWarning(
			"Database read error: %s",
			qPrintable(
				reader.getErrorString()
			)
		);
	}
	QVERIFY(
		!reader.hasError()
	);
	QList<Entry*> entries = db.data()->getRootGroup()->getEntries();
	QCOMPARE(
		entries.size(),
		1
	);
	Entry* entry = entries.at(
		0
	);
	QList<Entry*> historyItems = entry->getHistoryItems();
	QCOMPARE(
		historyItems.size(),
		1
	);
	Entry* historyItem = historyItems.at(
		0
	);
	QVERIFY(
		!entry->getUUID().isNull()
	);
	QVERIFY(
		!historyItem->getUUID().isNull()
	);
	QCOMPARE(
		historyItem->getUUID(),
		entry->getUUID()
	);
}

void TestKeePass2XmlReader::cleanupTestCase()
{
	delete m_db;
}
