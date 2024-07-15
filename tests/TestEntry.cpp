/*
 *  Copyright (C) 2013 Felix Geyer <debfx@fobos.de>
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
#include "TestEntry.h"
#include <QTest>
#include "core/Entry.h"
#include "crypto/Crypto.h"
QTEST_GUILESS_MAIN(
	TestEntry
)

void TestEntry::initTestCase()
{
	QVERIFY(
		Crypto::init()
	);
}

void TestEntry::testHistoryItemDeletion()
{
	Entry* entry = new Entry();
	QPointer<Entry> historyEntry = new Entry();
	entry->addHistoryItem(
		historyEntry
	);
	QCOMPARE(
		entry->getHistoryItems().size(),
		1
	);
	QList<Entry*> historyEntriesToRemove;
	historyEntriesToRemove.append(
		historyEntry
	);
	entry->removeHistoryItems(
		historyEntriesToRemove
	);
	QCOMPARE(
		entry->getHistoryItems().size(),
		0
	);
	QVERIFY(
		historyEntry.isNull()
	);
	delete entry;
}

void TestEntry::testCopyDataFrom()
{
	Entry* entry = new Entry();
	entry->setTitle(
		"testtitle"
	);
	entry->getAttributes()->set(
		"attr1",
		"abc"
	);
	entry->getAttributes()->set(
		"attr2",
		"def"
	);
	entry->getAttachments()->set(
		"test",
		"123"
	);
	entry->getAttachments()->set(
		"test2",
		"456"
	);
	Entry* entry2 = new Entry();
	entry2->copyDataFrom(
		entry
	);
	delete entry;
	QCOMPARE(
		entry2->getTitle(),
		QString("testtitle")
	);
	QCOMPARE(
		entry2->getAttributes()->getValue("attr1"),
		QString("abc")
	);
	QCOMPARE(
		entry2->getAttributes()->getValue("attr2"),
		QString("def")
	);
	QCOMPARE(
		entry2->getAttachments()->getKeys().size(),
		2
	);
	QCOMPARE(
		entry2->getAttachments()->getValue("test"),
		QByteArray("123")
	);
	QCOMPARE(
		entry2->getAttachments()->getValue("test2"),
		QByteArray("456")
	);
}

void TestEntry::testClone()
{
	Entry* entryOrg = new Entry();
	entryOrg->setUUID(
		UUID::random()
	);
	entryOrg->setTitle(
		"Original Title"
	);
	entryOrg->beginUpdate();
	entryOrg->setTitle(
		"New Title"
	);
	entryOrg->endUpdate();
	TimeInfo entryOrgTime = entryOrg->getTimeInfo();
	QDateTime dateTime;
	dateTime.setTimeSpec(
		Qt::UTC
	);
	dateTime.setSecsSinceEpoch(
		60
	);
	entryOrgTime.setCreationTime(
		dateTime
	);
	entryOrg->setTimeInfo(
		entryOrgTime
	);
	Entry* entryCloneNone = entryOrg->clone(
		Entry::CloneNoFlags
	);
	QCOMPARE(
		entryCloneNone->getUUID(),
		entryOrg->getUUID()
	);
	QCOMPARE(
		entryCloneNone->getTitle(),
		QString("New Title")
	);
	QCOMPARE(
		entryCloneNone->getHistoryItems().size(),
		0
	);
	QCOMPARE(
		entryCloneNone->getTimeInfo().getCreationTime(),
		entryOrg->getTimeInfo().getCreationTime()
	);
	Entry* entryCloneNewUuid = entryOrg->clone(
		Entry::CloneNewUuid
	);
	QVERIFY(
		entryCloneNewUuid->getUUID() != entryOrg->getUUID()
	);
	QVERIFY(
		!entryCloneNewUuid->getUUID().isNull()
	);
	QCOMPARE(
		entryCloneNewUuid->getTitle(),
		QString("New Title")
	);
	QCOMPARE(
		entryCloneNewUuid->getHistoryItems().size(),
		0
	);
	QCOMPARE(
		entryCloneNewUuid->getTimeInfo().getCreationTime(),
		entryOrg->getTimeInfo().getCreationTime()
	);
	Entry* entryCloneResetTime = entryOrg->clone(
		Entry::CloneResetTimeInfo
	);
	QCOMPARE(
		entryCloneNone->getUUID(),
		entryOrg->getUUID()
	);
	QCOMPARE(
		entryCloneResetTime->getTitle(),
		QString("New Title")
	);
	QCOMPARE(
		entryCloneResetTime->getHistoryItems().size(),
		0
	);
	QVERIFY(
		entryCloneResetTime->getTimeInfo().getCreationTime() != entryOrg->
		getTimeInfo(). getCreationTime()
	);
	Entry* entryCloneHistory = entryOrg->clone(
		Entry::CloneIncludeHistory
	);
	QCOMPARE(
		entryCloneNone->getUUID(),
		entryOrg->getUUID()
	);
	QCOMPARE(
		entryCloneHistory->getTitle(),
		QString("New Title")
	);
	QCOMPARE(
		entryCloneHistory->getHistoryItems().size(),
		1
	);
	QCOMPARE(
		entryCloneHistory->getHistoryItems().at(0)->getTitle(),
		QString("Original Title")
	);
	QCOMPARE(
		entryCloneHistory->getTimeInfo().getCreationTime(),
		entryOrg->getTimeInfo().getCreationTime()
	);
}
