/*
 *  Copyright (C) 2011 Felix Geyer <debfx@fobos.de>
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
#include "TestGuiPixmaps.h"
#include <QTest>
#include "core/Database.h"
#include "core/DatabaseIcons.h"
#include "core/Entry.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "crypto/Crypto.h"

void TestGuiPixmaps::initTestCase()
{
	QVERIFY(
		Crypto::init()
	);
}

void TestGuiPixmaps::testDatabaseIcons()
{
	QImage image;
	QPixmap pixmap;
	QPixmap pixmapCached;
	image = DatabaseIcons::getInstance()->icon(
		0
	);
	pixmap = DatabaseIcons::getInstance()->iconPixmap(
		0
	);
	compareImages(
		pixmap,
		image
	);
	// check if the cache works correctly
	pixmapCached = DatabaseIcons::getInstance()->iconPixmap(
		0
	);
	compareImages(
		pixmapCached,
		image
	);
	QCOMPARE(
		pixmapCached.cacheKey(),
		pixmap.cacheKey()
	);
	pixmap = DatabaseIcons::getInstance()->iconPixmap(
		1
	);
	image = DatabaseIcons::getInstance()->icon(
		1
	);
	compareImages(
		pixmap,
		image
	);
	pixmapCached = DatabaseIcons::getInstance()->iconPixmap(
		1
	);
	compareImages(
		pixmapCached,
		image
	);
	QCOMPARE(
		pixmapCached.cacheKey(),
		pixmap.cacheKey()
	);
}

void TestGuiPixmaps::testEntryIcons()
{
	Database* db = new Database();
	Entry* entry = new Entry();
	entry->setGroup(
		db->getRootGroup()
	);
	QImage icon;
	QImage image;
	QPixmap pixmap;
	QPixmap pixmapCached1;
	QPixmap pixmapCached2;
	icon = DatabaseIcons::getInstance()->icon(
		10
	);
	entry->setIcon(
		10
	);
	image = entry->getIcon();
	pixmap = entry->getIconPixmap();
	QCOMPARE(
		image,
		icon
	);
	compareImages(
		pixmap,
		icon
	);
	pixmapCached1 = entry->getIconPixmap();
	pixmapCached2 = DatabaseIcons::getInstance()->iconPixmap(
		10
	);
	compareImages(
		pixmapCached1,
		icon
	);
	compareImages(
		pixmapCached2,
		icon
	);
	QCOMPARE(
		pixmapCached1.cacheKey(),
		pixmap.cacheKey()
	);
	QCOMPARE(
		pixmapCached2.cacheKey(),
		pixmap.cacheKey()
	);
	UUID iconUuid = UUID::random();
	icon = QImage(
		2,
		1,
		QImage::Format_RGB32
	);
	icon.setPixel(
		0,
		0,
		qRgb(
			0,
			0,
			0
		)
	);
	icon.setPixel(
		1,
		0,
		qRgb(
			0,
			0,
			50
		)
	);
	db->getMetadata()->addCustomIcon(
		iconUuid,
		icon
	);
	entry->setIcon(
		iconUuid
	);
	image = entry->getIcon();
	pixmap = entry->getIconPixmap();
	QCOMPARE(
		image,
		icon
	);
	compareImages(
		pixmap,
		icon
	);
	pixmapCached1 = entry->getIconPixmap();
	compareImages(
		pixmapCached1,
		icon
	);
	QCOMPARE(
		pixmapCached1.cacheKey(),
		pixmap.cacheKey()
	);
	delete db;
}

void TestGuiPixmaps::testGroupIcons()
{
	Database* db = new Database();
	Group* group = db->getRootGroup();
	QImage icon;
	QImage image;
	QPixmap pixmap;
	QPixmap pixmapCached1;
	QPixmap pixmapCached2;
	icon = DatabaseIcons::getInstance()->icon(
		10
	);
	group->setIcon(
		10
	);
	image = group->getIcon();
	pixmap = group->getIconPixmap();
	QCOMPARE(
		image,
		icon
	);
	compareImages(
		pixmap,
		icon
	);
	pixmapCached1 = group->getIconPixmap();
	pixmapCached2 = DatabaseIcons::getInstance()->iconPixmap(
		10
	);
	compareImages(
		pixmapCached1,
		icon
	);
	compareImages(
		pixmapCached2,
		icon
	);
	QCOMPARE(
		pixmapCached1.cacheKey(),
		pixmap.cacheKey()
	);
	QCOMPARE(
		pixmapCached2.cacheKey(),
		pixmap.cacheKey()
	);
	UUID iconUuid = UUID::random();
	icon = QImage(
		2,
		1,
		QImage::Format_RGB32
	);
	icon.setPixel(
		0,
		0,
		qRgb(
			0,
			0,
			0
		)
	);
	icon.setPixel(
		1,
		0,
		qRgb(
			0,
			0,
			50
		)
	);
	db->getMetadata()->addCustomIcon(
		iconUuid,
		icon
	);
	group->setIcon(
		iconUuid
	);
	image = group->getIcon();
	pixmap = group->getIconPixmap();
	QCOMPARE(
		image,
		icon
	);
	compareImages(
		pixmap,
		icon
	);
	pixmapCached1 = group->getIconPixmap();
	compareImages(
		pixmapCached1,
		icon
	);
	QCOMPARE(
		pixmapCached1.cacheKey(),
		pixmap.cacheKey()
	);
	delete db;
}

void TestGuiPixmaps::compareImages(
	const QPixmap &pixmap,
	const QImage &image
)
{
	QCOMPARE(
		pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied),
		image.convertToFormat(QImage::Format_ARGB32_Premultiplied)
	);
}

QTEST_MAIN(
	TestGuiPixmaps
)
