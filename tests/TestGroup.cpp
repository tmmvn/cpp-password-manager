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
#include "TestGroup.h"
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include "core/Database.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "crypto/Crypto.h"
QTEST_GUILESS_MAIN(
	TestGroup
)

void TestGroup::initTestCase()
{
	qRegisterMetaType<Entry*>(
		"Entry*"
	);
	qRegisterMetaType<Group*>(
		"Group*"
	);
	QVERIFY(
		Crypto::init()
	);
}

void TestGroup::testParenting()
{
	Database* db = new Database();
	QPointer<Group> rootGroup = db->getRootGroup();
	Group* tmpRoot = new Group();
	QPointer<Group> g1 = new Group();
	QPointer<Group> g2 = new Group();
	QPointer<Group> g3 = new Group();
	QPointer<Group> g4 = new Group();
	g1->setParent(
		tmpRoot
	);
	g2->setParent(
		tmpRoot
	);
	g3->setParent(
		tmpRoot
	);
	g4->setParent(
		tmpRoot
	);
	g2->setParent(
		g1
	);
	g4->setParent(
		g3
	);
	g3->setParent(
		g1
	);
	g1->setParent(
		db->getRootGroup()
	);
	QVERIFY(
		g1->getParentGroup() == rootGroup
	);
	QVERIFY(
		g2->getParentGroup() == g1
	);
	QVERIFY(
		g3->getParentGroup() == g1
	);
	QVERIFY(
		g4->getParentGroup() == g3
	);
	QVERIFY(
		g1->getDatabase() == db
	);
	QVERIFY(
		g2->getDatabase() == db
	);
	QVERIFY(
		g3->getDatabase() == db
	);
	QVERIFY(
		g4->getDatabase() == db
	);
	QCOMPARE(
		tmpRoot->getChildren().size(),
		0
	);
	QCOMPARE(
		rootGroup->getChildren().size(),
		1
	);
	QCOMPARE(
		g1->getChildren().size(),
		2
	);
	QCOMPARE(
		g2->getChildren().size(),
		0
	);
	QCOMPARE(
		g3->getChildren().size(),
		1
	);
	QCOMPARE(
		g4->getChildren().size(),
		0
	);
	QVERIFY(
		rootGroup->getChildren().at(0) == g1
	);
	QVERIFY(
		g1->getChildren().at(0) == g2
	);
	QVERIFY(
		g1->getChildren().at(1) == g3
	);
	QVERIFY(
		g3->getChildren().contains(g4)
	);
	Group* g5 = new Group();
	Group* g6 = new Group();
	g5->setParent(
		db->getRootGroup()
	);
	g6->setParent(
		db->getRootGroup()
	);
	QVERIFY(
		db->getRootGroup()->getChildren().at(1) == g5
	);
	QVERIFY(
		db->getRootGroup()->getChildren().at(2) == g6
	);
	g5->setParent(
		db->getRootGroup()
	);
	QVERIFY(
		db->getRootGroup()->getChildren().at(1) == g6
	);
	QVERIFY(
		db->getRootGroup()->getChildren().at(2) == g5
	);
	QSignalSpy spy(
		db,
		SIGNAL(
			groupDataChanged(Group*)
		)
	);
	g2->setName(
		"test"
	);
	g4->setName(
		"test"
	);
	g3->setName(
		"test"
	);
	g1->setName(
		"test"
	);
	g3->setIcon(
		UUID::random()
	);
	g1->setIcon(
		2
	);
	QCOMPARE(
		spy.count(),
		6
	);
	delete db;
	QVERIFY(
		rootGroup.isNull()
	);
	QVERIFY(
		g1.isNull()
	);
	QVERIFY(
		g2.isNull()
	);
	QVERIFY(
		g3.isNull()
	);
	QVERIFY(
		g4.isNull()
	);
	delete tmpRoot;
}

void TestGroup::testSignals()
{
	Database* db = new Database();
	Database* db2 = new Database();
	QPointer<Group> root = db->getRootGroup();
	QSignalSpy spyAboutToAdd(
		db,
		SIGNAL(
			groupAboutToAdd(Group*,int)
		)
	);
	QSignalSpy spyAdded(
		db,
		SIGNAL(
			groupAdded()
		)
	);
	QSignalSpy spyAboutToRemove(
		db,
		SIGNAL(
			groupAboutToRemove(Group*)
		)
	);
	QSignalSpy spyRemoved(
		db,
		SIGNAL(
			groupRemoved()
		)
	);
	QSignalSpy spyAboutToMove(
		db,
		SIGNAL(
			groupAboutToMove(Group*,Group*,int)
		)
	);
	QSignalSpy spyMoved(
		db,
		SIGNAL(
			groupMoved()
		)
	);
	QSignalSpy spyAboutToAdd2(
		db2,
		SIGNAL(
			groupAboutToAdd(Group*,int)
		)
	);
	QSignalSpy spyAdded2(
		db2,
		SIGNAL(
			groupAdded()
		)
	);
	QSignalSpy spyAboutToRemove2(
		db2,
		SIGNAL(
			groupAboutToRemove(Group*)
		)
	);
	QSignalSpy spyRemoved2(
		db2,
		SIGNAL(
			groupRemoved()
		)
	);
	QSignalSpy spyAboutToMove2(
		db2,
		SIGNAL(
			groupAboutToMove(Group*,Group*,int)
		)
	);
	QSignalSpy spyMoved2(
		db2,
		SIGNAL(
			groupMoved()
		)
	);
	Group* g1 = new Group();
	Group* g2 = new Group();
	g1->setParent(
		root
	);
	QCOMPARE(
		spyAboutToAdd.count(),
		1
	);
	QCOMPARE(
		spyAdded.count(),
		1
	);
	QCOMPARE(
		spyAboutToRemove.count(),
		0
	);
	QCOMPARE(
		spyRemoved.count(),
		0
	);
	QCOMPARE(
		spyAboutToMove.count(),
		0
	);
	QCOMPARE(
		spyMoved.count(),
		0
	);
	g2->setParent(
		root
	);
	QCOMPARE(
		spyAboutToAdd.count(),
		2
	);
	QCOMPARE(
		spyAdded.count(),
		2
	);
	QCOMPARE(
		spyAboutToRemove.count(),
		0
	);
	QCOMPARE(
		spyRemoved.count(),
		0
	);
	QCOMPARE(
		spyAboutToMove.count(),
		0
	);
	QCOMPARE(
		spyMoved.count(),
		0
	);
	g2->setParent(
		root
	);
	QCOMPARE(
		spyAboutToAdd.count(),
		2
	);
	QCOMPARE(
		spyAdded.count(),
		2
	);
	QCOMPARE(
		spyAboutToRemove.count(),
		0
	);
	QCOMPARE(
		spyRemoved.count(),
		0
	);
	QCOMPARE(
		spyAboutToMove.count(),
		0
	);
	QCOMPARE(
		spyMoved.count(),
		0
	);
	g2->setParent(
		root,
		0
	);
	QCOMPARE(
		spyAboutToAdd.count(),
		2
	);
	QCOMPARE(
		spyAdded.count(),
		2
	);
	QCOMPARE(
		spyAboutToRemove.count(),
		0
	);
	QCOMPARE(
		spyRemoved.count(),
		0
	);
	QCOMPARE(
		spyAboutToMove.count(),
		1
	);
	QCOMPARE(
		spyMoved.count(),
		1
	);
	g1->setParent(
		g2
	);
	QCOMPARE(
		spyAboutToAdd.count(),
		2
	);
	QCOMPARE(
		spyAdded.count(),
		2
	);
	QCOMPARE(
		spyAboutToRemove.count(),
		0
	);
	QCOMPARE(
		spyRemoved.count(),
		0
	);
	QCOMPARE(
		spyAboutToMove.count(),
		2
	);
	QCOMPARE(
		spyMoved.count(),
		2
	);
	delete g1;
	QCOMPARE(
		spyAboutToAdd.count(),
		2
	);
	QCOMPARE(
		spyAdded.count(),
		2
	);
	QCOMPARE(
		spyAboutToRemove.count(),
		1
	);
	QCOMPARE(
		spyRemoved.count(),
		1
	);
	QCOMPARE(
		spyAboutToMove.count(),
		2
	);
	QCOMPARE(
		spyMoved.count(),
		2
	);
	g2->setParent(
		db2->getRootGroup()
	);
	QCOMPARE(
		spyAboutToAdd.count(),
		2
	);
	QCOMPARE(
		spyAdded.count(),
		2
	);
	QCOMPARE(
		spyAboutToRemove.count(),
		2
	);
	QCOMPARE(
		spyRemoved.count(),
		2
	);
	QCOMPARE(
		spyAboutToMove.count(),
		2
	);
	QCOMPARE(
		spyMoved.count(),
		2
	);
	QCOMPARE(
		spyAboutToAdd2.count(),
		1
	);
	QCOMPARE(
		spyAdded2.count(),
		1
	);
	QCOMPARE(
		spyAboutToRemove2.count(),
		0
	);
	QCOMPARE(
		spyRemoved2.count(),
		0
	);
	QCOMPARE(
		spyAboutToMove2.count(),
		0
	);
	QCOMPARE(
		spyMoved2.count(),
		0
	);
	Group* g3 = new Group();
	Group* g4 = new Group();
	g3->setParent(
		root
	);
	QCOMPARE(
		spyAboutToAdd.count(),
		3
	);
	QCOMPARE(
		spyAdded.count(),
		3
	);
	QCOMPARE(
		spyAboutToRemove.count(),
		2
	);
	QCOMPARE(
		spyRemoved.count(),
		2
	);
	QCOMPARE(
		spyAboutToMove.count(),
		2
	);
	QCOMPARE(
		spyMoved.count(),
		2
	);
	g4->setParent(
		root
	);
	QCOMPARE(
		spyAboutToAdd.count(),
		4
	);
	QCOMPARE(
		spyAdded.count(),
		4
	);
	QCOMPARE(
		spyAboutToRemove.count(),
		2
	);
	QCOMPARE(
		spyRemoved.count(),
		2
	);
	QCOMPARE(
		spyAboutToMove.count(),
		2
	);
	QCOMPARE(
		spyMoved.count(),
		2
	);
	g3->setParent(
		root
	);
	QCOMPARE(
		spyAboutToAdd.count(),
		4
	);
	QCOMPARE(
		spyAdded.count(),
		4
	);
	QCOMPARE(
		spyAboutToRemove.count(),
		2
	);
	QCOMPARE(
		spyRemoved.count(),
		2
	);
	QCOMPARE(
		spyAboutToMove.count(),
		3
	);
	QCOMPARE(
		spyMoved.count(),
		3
	);
	delete db;
	delete db2;
	QVERIFY(
		root.isNull()
	);
}

void TestGroup::testEntries()
{
	Group* group = new Group();
	QPointer<Entry> entry1 = new Entry();
	entry1->setGroup(
		group
	);
	QPointer<Entry> entry2 = new Entry();
	entry2->setGroup(
		group
	);
	QCOMPARE(
		group->getEntries().size(),
		2
	);
	QVERIFY(
		group->getEntries().at(0) == entry1
	);
	QVERIFY(
		group->getEntries().at(1) == entry2
	);
	delete group;
	QVERIFY(
		entry1.isNull()
	);
	QVERIFY(
		entry2.isNull()
	);
}

void TestGroup::testDeleteSignals()
{
	Database* db = new Database();
	Group* groupRoot = db->getRootGroup();
	Group* groupChild = new Group();
	Group* groupChildChild = new Group();
	groupRoot->setObjectName(
		"groupRoot"
	);
	groupChild->setObjectName(
		"groupChild"
	);
	groupChildChild->setObjectName(
		"groupChildChild"
	);
	groupChild->setParent(
		groupRoot
	);
	groupChildChild->setParent(
		groupChild
	);
	QSignalSpy spyAboutToRemove(
		db,
		SIGNAL(
			groupAboutToRemove(Group*)
		)
	);
	QSignalSpy spyRemoved(
		db,
		SIGNAL(
			groupRemoved()
		)
	);
	delete groupChild;
	QVERIFY(
		groupRoot->getChildren().isEmpty()
	);
	QCOMPARE(
		spyAboutToRemove.count(),
		2
	);
	QCOMPARE(
		spyRemoved.count(),
		2
	);
	delete db;
	Group* group = new Group();
	Entry* entry = new Entry();
	entry->setGroup(
		group
	);
	QSignalSpy spyEntryAboutToRemove(
		group,
		SIGNAL(
			entryAboutToRemove(Entry*)
		)
	);
	QSignalSpy spyEntryRemoved(
		group,
		SIGNAL(
			entryRemoved(Entry*)
		)
	);
	delete entry;
	QVERIFY(
		group->getEntries().isEmpty()
	);
	QCOMPARE(
		spyEntryAboutToRemove.count(),
		1
	);
	QCOMPARE(
		spyEntryRemoved.count(),
		1
	);
	delete group;
	Database* db2 = new Database();
	Group* groupRoot2 = db2->getRootGroup();
	Group* group2 = new Group();
	group2->setParent(
		groupRoot2
	);
	Entry* entry2 = new Entry();
	entry2->setGroup(
		group2
	);
	QSignalSpy spyEntryAboutToRemove2(
		group2,
		SIGNAL(
			entryAboutToRemove(Entry*)
		)
	);
	QSignalSpy spyEntryRemoved2(
		group2,
		SIGNAL(
			entryRemoved(Entry*)
		)
	);
	delete group2;
	QCOMPARE(
		spyEntryAboutToRemove2.count(),
		1
	);
	QCOMPARE(
		spyEntryRemoved2.count(),
		1
	);
	delete db2;
}

void TestGroup::testCopyCustomIcon()
{
	Database* dbSource = new Database();
	UUID groupIconUuid = UUID::random();
	QImage groupIcon(
		16,
		16,
		QImage::Format_RGB32
	);
	groupIcon.setPixel(
		0,
		0,
		qRgb(
			255,
			0,
			0
		)
	);
	dbSource->getMetadata()->addCustomIcon(
		groupIconUuid,
		groupIcon
	);
	UUID entryIconUuid = UUID::random();
	QImage entryIcon(
		16,
		16,
		QImage::Format_RGB32
	);
	entryIcon.setPixel(
		0,
		0,
		qRgb(
			255,
			0,
			0
		)
	);
	dbSource->getMetadata()->addCustomIcon(
		entryIconUuid,
		entryIcon
	);
	Group* group = new Group();
	group->setParent(
		dbSource->getRootGroup()
	);
	group->setIcon(
		groupIconUuid
	);
	QCOMPARE(
		group->getIcon(),
		groupIcon
	);
	Entry* entry = new Entry();
	entry->setGroup(
		dbSource->getRootGroup()
	);
	entry->setIcon(
		entryIconUuid
	);
	QCOMPARE(
		entry->getIcon(),
		entryIcon
	);
	Database* dbTarget = new Database();
	group->setParent(
		dbTarget->getRootGroup()
	);
	QVERIFY(
		dbTarget->getMetadata()->containsCustomIcon(groupIconUuid)
	);
	QCOMPARE(
		dbTarget->getMetadata()->getCustomIcon(groupIconUuid),
		groupIcon
	);
	QCOMPARE(
		group->getIcon(),
		groupIcon
	);
	entry->setGroup(
		dbTarget->getRootGroup()
	);
	QVERIFY(
		dbTarget->getMetadata()->containsCustomIcon(entryIconUuid)
	);
	QCOMPARE(
		dbTarget->getMetadata()->getCustomIcon(entryIconUuid),
		entryIcon
	);
	QCOMPARE(
		entry->getIcon(),
		entryIcon
	);
	delete dbSource;
	delete dbTarget;
}

void TestGroup::testClone()
{
	Database* db = new Database();
	Group* originalGroup = new Group();
	originalGroup->setParent(
		db->getRootGroup()
	);
	originalGroup->setName(
		"Group"
	);
	originalGroup->setIcon(
		42
	);
	Entry* originalGroupEntry = new Entry();
	originalGroupEntry->setGroup(
		originalGroup
	);
	originalGroupEntry->setTitle(
		"GroupEntryOld"
	);
	originalGroupEntry->setIcon(
		43
	);
	originalGroupEntry->beginUpdate();
	originalGroupEntry->setTitle(
		"GroupEntry"
	);
	originalGroupEntry->endUpdate();
	Group* subGroup = new Group();
	subGroup->setParent(
		originalGroup
	);
	subGroup->setName(
		"SubGroup"
	);
	Entry* subGroupEntry = new Entry();
	subGroupEntry->setGroup(
		subGroup
	);
	subGroupEntry->setTitle(
		"SubGroupEntry"
	);
	Group* clonedGroup = originalGroup->clone();
	QVERIFY(
		!clonedGroup->getParentGroup()
	);
	QVERIFY(
		!clonedGroup->getDatabase()
	);
	QVERIFY(
		clonedGroup->getUUID() != originalGroup->getUUID()
	);
	QCOMPARE(
		clonedGroup->getName(),
		QString("Group")
	);
	QCOMPARE(
		clonedGroup->getIconNumber(),
		42
	);
	QCOMPARE(
		clonedGroup->getChildren().size(),
		1
	);
	QCOMPARE(
		clonedGroup->getEntries().size(),
		1
	);
	Entry* clonedGroupEntry = clonedGroup->getEntries().at(
		0
	);
	QVERIFY(
		clonedGroupEntry->getUUID() != originalGroupEntry->getUUID()
	);
	QCOMPARE(
		clonedGroupEntry->getTitle(),
		QString("GroupEntry")
	);
	QCOMPARE(
		clonedGroupEntry->getIconNumber(),
		43
	);
	QCOMPARE(
		clonedGroupEntry->getHistoryItems().size(),
		0
	);
	Group* clonedSubGroup = clonedGroup->getChildren().at(
		0
	);
	QVERIFY(
		clonedSubGroup->getUUID() != subGroup->getUUID()
	);
	QCOMPARE(
		clonedSubGroup->getName(),
		QString("SubGroup")
	);
	QCOMPARE(
		clonedSubGroup->getChildren().size(),
		0
	);
	QCOMPARE(
		clonedSubGroup->getEntries().size(),
		1
	);
	Entry* clonedSubGroupEntry = clonedSubGroup->getEntries().at(
		0
	);
	QVERIFY(
		clonedSubGroupEntry->getUUID() != subGroupEntry->getUUID()
	);
	QCOMPARE(
		clonedSubGroupEntry->getTitle(),
		QString("SubGroupEntry")
	);
	Group* clonedGroupKeepUuid = originalGroup->clone(
		Entry::CloneNoFlags
	);
	QCOMPARE(
		clonedGroupKeepUuid->getEntries().at(0)->getUUID(),
		originalGroupEntry->getUUID()
	);
	QCOMPARE(
		clonedGroupKeepUuid->getChildren().at(0)->getEntries().at(0)->getUUID(),
		subGroupEntry->getUUID()
	);
	delete clonedGroup;
	delete clonedGroupKeepUuid;
	delete db;
}

void TestGroup::testCopyCustomIcons()
{
	Database* dbSource = new Database();
	Database* dbTarget = new Database();
	QImage iconImage1(
		1,
		1,
		QImage::Format_RGB32
	);
	iconImage1.setPixel(
		0,
		0,
		qRgb(
			1,
			2,
			3
		)
	);
	QImage iconImage2(
		1,
		1,
		QImage::Format_RGB32
	);
	iconImage2.setPixel(
		0,
		0,
		qRgb(
			4,
			5,
			6
		)
	);
	Group* group1 = new Group();
	group1->setParent(
		dbSource->getRootGroup()
	);
	UUID group1Icon = UUID::random();
	dbSource->getMetadata()->addCustomIcon(
		group1Icon,
		iconImage1
	);
	group1->setIcon(
		group1Icon
	);
	Group* group2 = new Group();
	group2->setParent(
		group1
	);
	UUID group2Icon = UUID::random();
	dbSource->getMetadata()->addCustomIcon(
		group2Icon,
		iconImage1
	);
	group2->setIcon(
		group2Icon
	);
	Entry* entry1 = new Entry();
	entry1->setGroup(
		group2
	);
	UUID entry1IconOld = UUID::random();
	dbSource->getMetadata()->addCustomIcon(
		entry1IconOld,
		iconImage1
	);
	entry1->setIcon(
		entry1IconOld
	);
	// add history item
	entry1->beginUpdate();
	UUID entry1IconNew = UUID::random();
	dbSource->getMetadata()->addCustomIcon(
		entry1IconNew,
		iconImage1
	);
	entry1->setIcon(
		entry1IconNew
	);
	entry1->endUpdate();
	// test that we don't overwrite icons
	dbTarget->getMetadata()->addCustomIcon(
		group2Icon,
		iconImage2
	);
	dbTarget->getMetadata()->copyCustomIcons(
		group1->getCustomIconsRecursive(),
		dbSource->getMetadata()
	);
	Metadata* metaTarget = dbTarget->getMetadata();
	QCOMPARE(
		metaTarget->getCustomIcons().size(),
		4
	);
	QVERIFY(
		metaTarget->containsCustomIcon(group1Icon)
	);
	QVERIFY(
		metaTarget->containsCustomIcon(group2Icon)
	);
	QVERIFY(
		metaTarget->containsCustomIcon(entry1IconOld)
	);
	QVERIFY(
		metaTarget->containsCustomIcon(entry1IconNew)
	);
	QCOMPARE(
		metaTarget->getCustomIcon(group1Icon).pixel(0, 0),
		qRgb(1, 2, 3)
	);
	QCOMPARE(
		metaTarget->getCustomIcon(group2Icon).pixel(0, 0),
		qRgb(4, 5, 6)
	);
}
