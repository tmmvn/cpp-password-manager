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
#include "TestKeePass2Writer.h"
#include <QBuffer>
#include <QFile>
#include <QTest>
#include "config-keepassx-tests.h"
#include "FailDevice.h"
#include "core/Database.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "crypto/Crypto.h"
#include "format/KeePass2Reader.h"
#include "format/KeePass2Repair.h"
#include "format/KeePass2Writer.h"
#include "format/KeePass2XmlWriter.h"
#include "keys/PasswordKey.h"
QTEST_GUILESS_MAIN(
	TestKeePass2Writer
)

void TestKeePass2Writer::initTestCase()
{
	qDebug() << "Init crypto";
	QVERIFY(
		Crypto::init()
	);
	qDebug() << "Add key";
	CompositeKey key;
	key.addKey(
		PasswordKey(
			"test"
		)
	);
	qDebug() << "New db";
	m_dbOrg = new Database();
	qDebug() << "Set db key";
	m_dbOrg->setKey(
		key
	);
	qDebug() << "Get metadata set name";
	m_dbOrg->getMetadata()->setName(
		"TESTDB"
	);
	qDebug() << "Get group";
	Group* group = m_dbOrg->getRootGroup();
	qDebug() << "Set group uuid";
	group->setUuid(
		UUID::random()
	);
	qDebug() << "Set notes";
	group->setNotes(
		"I'm a note!"
	);
	qDebug() << "New entry";
	Entry* entry = new Entry();
	entry->setPassword(
		QString::fromUtf8(
			"\xc3\xa4\xa3\xb6\xc3\xbc\xe9\x9b\xbb\xe7\xb4\x85"
		)
	);
	entry->setUUID(
		UUID::random()
	);
	entry->getAttributes()->set(
		"test",
		"protectedTest",
		true
	);
	QVERIFY(
		entry->getAttributes()->isProtected("test")
	);
	entry->getAttachments()->set(
		"myattach.txt",
		QByteArray(
			"this is an attachment"
		)
	);
	entry->getAttachments()->set(
		"aaa.txt",
		QByteArray(
			"also an attachment"
		)
	);
	entry->setGroup(
		group
	);
	qDebug() << "New group";
	Group* groupNew = new Group();
	groupNew->setUuid(
		UUID::random()
	);
	groupNew->setName(
		"TESTGROUP"
	);
	groupNew->setNotes(
		"I'm a sub group note!"
	);
	groupNew->setParent(
		group
	);
	qDebug() << "New buffer";
	QBuffer buffer;
	buffer.open(
		QBuffer::ReadWrite
	);
	qDebug() << "New writer";
	KeePass2Writer writer;
	writer.writeDatabase(
		&buffer,
		m_dbOrg
	);
	QVERIFY(
		!writer.hasError()
	);
	buffer.seek(
		0
	);
	qDebug() << "Reader";
	KeePass2Reader reader;
	m_dbTest = reader.readDatabase(
		&buffer,
		key
	);
	QVERIFY(
		!reader.hasError()
	);
	QVERIFY(
		m_dbTest
	);
}

void TestKeePass2Writer::testBasic()
{
	QCOMPARE(
		m_dbTest->getMetadata()->getName(),
		m_dbOrg->getMetadata()->getName()
	);
	QVERIFY(
		m_dbTest->getRootGroup()
	);
	QCOMPARE(
		m_dbTest->getRootGroup()->getChildren()[0]->getName(),
		m_dbOrg->getRootGroup()->getChildren()[0]->getName()
	);
	QCOMPARE(
		m_dbTest->getRootGroup()->getNotes(),
		m_dbOrg->getRootGroup()->getNotes()
	);
	QCOMPARE(
		m_dbTest->getRootGroup()->getChildren()[0]->getNotes(),
		m_dbOrg->getRootGroup()->getChildren()[0]->getNotes()
	);
}

void TestKeePass2Writer::testProtectedAttributes()
{
	QCOMPARE(
		m_dbTest->getRootGroup()->getEntries().size(),
		1
	);
	Entry* entry = m_dbTest->getRootGroup()->getEntries().at(
		0
	);
	QCOMPARE(
		entry->getAttributes()->getValue("test"),
		QString("protectedTest")
	);
	QCOMPARE(
		entry->getAttributes()->isProtected("test"),
		true
	);
}

void TestKeePass2Writer::testAttachments()
{
	Entry* entry = m_dbTest->getRootGroup()->getEntries().at(
		0
	);
	QCOMPARE(
		entry->getAttachments()->getKeys().size(),
		2
	);
	QCOMPARE(
		entry->getAttachments()->getValue("myattach.txt"),
		QByteArray("this is an attachment")
	);
	QCOMPARE(
		entry->getAttachments()->getValue("aaa.txt"),
		QByteArray("also an attachment")
	);
}

void TestKeePass2Writer::testNonAsciiPasswords()
{
	QCOMPARE(
		m_dbTest->getRootGroup()->getEntries()[0]->getPassword(),
		m_dbOrg->getRootGroup()->getEntries()[0]->getPassword()
	);
}

void TestKeePass2Writer::testDeviceFailure()
{
	CompositeKey key;
	key.addKey(
		PasswordKey(
			"test"
		)
	);
	Database* db = new Database();
	db->setKey(
		key
	);
	// Disable compression so we write a predictable number of bytes.
	db->setCompressionAlgo(
		Database::CompressionNone
	);
	Entry* entry = new Entry();
	entry->setParent(
		db->getRootGroup()
	);
	QByteArray attachment(
		4096,
		'Z'
	);
	entry->getAttachments()->set(
		"test",
		attachment
	);
	FailDevice failDevice(
		512
	);
	QVERIFY(
		failDevice.open(QIODevice::WriteOnly)
	);
	KeePass2Writer writer;
	writer.writeDatabase(
		&failDevice,
		db
	);
	QVERIFY(
		writer.hasError()
	);
	QCOMPARE(
		writer.getErrorString(),
		QString("FAILDEVICE")
	);
	delete db;
}

void TestKeePass2Writer::testRepair()
{
	QString brokenDbFilename = QString(
		KEEPASSX_TEST_DATA_DIR
	).append(
		"/bug392.kdbx"
	);
	// master password = test
	// entry username: testuser\x10\x20AC
	// entry password: testpw
	CompositeKey key;
	key.addKey(
		PasswordKey(
			"test"
		)
	);
	// test that we can't open the broken database
	KeePass2Reader reader;
	Database* dbBroken = reader.readDatabase(
		brokenDbFilename,
		key
	);
	QVERIFY(
		!dbBroken
	);
	QVERIFY(
		reader.hasError()
	);
	// test if we can repair the database
	KeePass2Repair repair;
	QFile file(
		brokenDbFilename
	);
	file.open(
		QIODevice::ReadOnly
	);
	QCOMPARE(
		repair.repairDatabase(&file, key),
		KeePass2Repair::RepairSuccess
	);
	Database* dbRepaired = repair.getDatabase();
	QVERIFY(
		dbRepaired
	);
	QCOMPARE(
		dbRepaired->getRootGroup()->getEntries().size(),
		1
	);
	QCOMPARE(
		dbRepaired->getRootGroup()->getEntries().at(0)->getUsername(),
		QString("testuser").append(QChar(0x20AC))
	);
	QCOMPARE(
		dbRepaired->getRootGroup()->getEntries().at(0)->getPassword(),
		QString("testpw")
	);
}

void TestKeePass2Writer::cleanupTestCase()
{
	delete m_dbOrg;
	delete m_dbTest;
}
