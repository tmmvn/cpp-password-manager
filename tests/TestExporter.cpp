/*
 *  Copyright (C) 2014 Felix Geyer <debfx@fobos.de>
 *  Copyright (C) 2014 Florian Geyer <blueice@fobos.de>
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
#include "TestExporter.h"
#include <QTest>
#include "core/ToDbExporter.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "crypto/Crypto.h"
QTEST_GUILESS_MAIN(
	TestExporter
)

void TestExporter::initTestCase()
{
	QVERIFY(
		Crypto::init()
	);
}

void TestExporter::testToDbExporter()
{
	QImage iconImage(
		1,
		1,
		QImage::Format_RGB32
	);
	iconImage.setPixel(
		0,
		0,
		qRgb(
			1,
			2,
			3
		)
	);
	UUID iconUuid = UUID::random();
	QImage iconUnusedImage(
		1,
		1,
		QImage::Format_RGB32
	);
	iconUnusedImage.setPixel(
		0,
		0,
		qRgb(
			1,
			2,
			3
		)
	);
	UUID iconUnusedUuid = UUID::random();
	Database* dbOrg = new Database();
	Group* groupOrg = new Group();
	groupOrg->setParent(
		dbOrg->getRootGroup()
	);
	groupOrg->setName(
		"GTEST"
	);
	Entry* entryOrg = new Entry();
	entryOrg->setGroup(
		groupOrg
	);
	entryOrg->setTitle(
		"ETEST"
	);
	dbOrg->getMetadata()->addCustomIcon(
		iconUuid,
		iconImage
	);
	dbOrg->getMetadata()->addCustomIcon(
		iconUnusedUuid,
		iconUnusedImage
	);
	entryOrg->setIcon(
		iconUuid
	);
	entryOrg->beginUpdate();
	entryOrg->setIcon(
		Entry::DefaultIconNumber
	);
	entryOrg->endUpdate();
	Database* dbExp = ToDbExporter().exportGroup(
		groupOrg
	);
	QCOMPARE(
		dbExp->getRootGroup()->getChildren().size(),
		1
	);
	Group* groupExp = dbExp->getRootGroup()->getChildren().at(
		0
	);
	QVERIFY(
		groupExp != groupOrg
	);
	QCOMPARE(
		groupExp->getName(),
		groupOrg->getName()
	);
	QCOMPARE(
		groupExp->getEntries().size(),
		1
	);
	Entry* entryExp = groupExp->getEntries().at(
		0
	);
	QCOMPARE(
		entryExp->getTitle(),
		entryOrg->getTitle()
	);
	QCOMPARE(
		dbExp->getMetadata()->getCustomIcons().size(),
		1
	);
	QVERIFY(
		dbExp->getMetadata()->containsCustomIcon(iconUuid)
	);
	QCOMPARE(
		entryExp->getIconNumber(),
		entryOrg->getIconNumber()
	);
	QCOMPARE(
		entryExp->getHistoryItems().size(),
		1
	);
	QCOMPARE(
		entryExp->getHistoryItems().at(0)->getIconUUID(),
		iconUuid
	);
	delete dbOrg;
	delete dbExp;
}
