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
#include "TestKeePass2Reader.h"
#include <QTest>
#include "config-keepassx-tests.h"
#include "core/Database.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "crypto/Crypto.h"
#include "format/KeePass2Reader.h"
#include "keys/PasswordKey.h"
QTEST_GUILESS_MAIN(
	TestKeePass2Reader
)

void TestKeePass2Reader::initTestCase()
{
	QVERIFY(
		Crypto::init()
	);
}

void TestKeePass2Reader::testNonAscii()
{
	QString filename = QString(
		KEEPASSX_TEST_DATA_DIR
	).append(
		"/NonAscii.kdbx"
	);
	CompositeKey key;
	key.addKey(
		PasswordKey(
			QString::fromUtf8(
				"\xce\x94\xc3\xb6\xd8\xb6"
			)
		)
	);
	KeePass2Reader reader;
	Database* db = reader.readDatabase(
		filename,
		key
	);
	QVERIFY(
		db
	);
	QVERIFY(
		!reader.hasError()
	);
	QCOMPARE(
		db->getMetadata()->getName(),
		QString("NonAsciiTest")
	);
	QCOMPARE(
		db->getCompressionAlgo(),
		Database::CompressionNone
	);
	delete db;
}

void TestKeePass2Reader::testCompressed()
{
	QString filename = QString(
		KEEPASSX_TEST_DATA_DIR
	).append(
		"/Compressed.kdbx"
	);
	CompositeKey key;
	key.addKey(
		PasswordKey(
			""
		)
	);
	KeePass2Reader reader;
	Database* db = reader.readDatabase(
		filename,
		key
	);
	QVERIFY(
		db
	);
	QVERIFY(
		!reader.hasError()
	);
	QCOMPARE(
		db->getMetadata()->getName(),
		QString("Compressed")
	);
	QCOMPARE(
		db->getCompressionAlgo(),
		Database::CompressionGZip
	);
	delete db;
}

void TestKeePass2Reader::testProtectedStrings()
{
	QString filename = QString(
		KEEPASSX_TEST_DATA_DIR
	).append(
		"/ProtectedStrings.kdbx"
	);
	CompositeKey key;
	key.addKey(
		PasswordKey(
			"masterpw"
		)
	);
	KeePass2Reader reader;
	Database* db = reader.readDatabase(
		filename,
		key
	);
	QVERIFY(
		db
	);
	QVERIFY(
		!reader.hasError()
	);
	QCOMPARE(
		db->getMetadata()->getName(),
		QString("Protected Strings Test")
	);
	Entry* entry = db->getRootGroup()->getEntries().at(
		0
	);
	QCOMPARE(
		entry->getTitle(),
		QString("Sample Entry")
	);
	QCOMPARE(
		entry->getUsername(),
		QString("Protected User Name")
	);
	QCOMPARE(
		entry->getPassword(),
		QString("ProtectedPassword")
	);
	QCOMPARE(
		entry->getAttributes()->getValue("TestProtected"),
		QString("ABC")
	);
	QCOMPARE(
		entry->getAttributes()->getValue("TestUnprotected"),
		QString("DEF")
	);
	QVERIFY(
		db->getMetadata()->protectPassword()
	);
	QVERIFY(
		entry->getAttributes()->isProtected("TestProtected")
	);
	QVERIFY(
		!entry->getAttributes()->isProtected("TestUnprotected")
	);
	delete db;
}

void TestKeePass2Reader::testBrokenHeaderHash()
{
	// The protected stream key has been modified in the header.
	// Make sure the database won't open.
	QString filename = QString(
		KEEPASSX_TEST_DATA_DIR
	).append(
		"/BrokenHeaderHash.kdbx"
	);
	CompositeKey key;
	key.addKey(
		PasswordKey(
			""
		)
	);
	KeePass2Reader reader;
	Database* db = reader.readDatabase(
		filename,
		key
	);
	QVERIFY(
		!db
	);
	QVERIFY(
		reader.hasError()
	);
	delete db;
}

void TestKeePass2Reader::testFormat200()
{
	QString filename = QString(
		KEEPASSX_TEST_DATA_DIR
	).append(
		"/Format200.kdbx"
	);
	CompositeKey key;
	key.addKey(
		PasswordKey(
			"a"
		)
	);
	KeePass2Reader reader;
	Database* db = reader.readDatabase(
		filename,
		key
	);
	QVERIFY(
		db
	);
	QVERIFY(
		!reader.hasError()
	);
	QCOMPARE(
		db->getRootGroup()->getName(),
		QString("Format200")
	);
	QVERIFY(
		!db->getMetadata()->protectTitle()
	);
	QVERIFY(
		db->getMetadata()->protectUsername()
	);
	QVERIFY(
		!db->getMetadata()->protectPassword()
	);
	QVERIFY(
		db->getMetadata()->protectUrl()
	);
	QVERIFY(
		!db->getMetadata()->protectNotes()
	);
	QCOMPARE(
		db->getRootGroup()->getEntries().size(),
		1
	);
	Entry* entry = db->getRootGroup()->getEntries().at(
		0
	);
	QCOMPARE(
		entry->getTitle(),
		QString("Sample Entry")
	);
	QCOMPARE(
		entry->getUsername(),
		QString("User Name")
	);
	QCOMPARE(
		entry->getAttachments()->getKeys().size(),
		2
	);
	QCOMPARE(
		entry->getAttachments()->getValue("myattach.txt"),
		QByteArray("abcdefghijk")
	);
	QCOMPARE(
		entry->getAttachments()->getValue("test.txt"),
		QByteArray("this is a test")
	);
	QCOMPARE(
		entry->getHistoryItems().size(),
		2
	);
	QCOMPARE(
		entry->getHistoryItems().at(0)->getAttachments()->getKeys().size(),
		0
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
	delete db;
}

void TestKeePass2Reader::testFormat300()
{
	QString filename = QString(
		KEEPASSX_TEST_DATA_DIR
	).append(
		"/Format300.kdbx"
	);
	CompositeKey key;
	key.addKey(
		PasswordKey(
			"a"
		)
	);
	KeePass2Reader reader;
	Database* db = reader.readDatabase(
		filename,
		key
	);
	QVERIFY(
		db
	);
	QVERIFY(
		!reader.hasError()
	);
	QCOMPARE(
		db->getRootGroup()->getName(),
		QString("Format300")
	);
	QCOMPARE(
		db->getMetadata()->getName(),
		QString("Test Database Format 0x00030000")
	);
	delete db;
}
