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
#ifndef KEEPASSX_METADATA_H
#define KEEPASSX_METADATA_H
#include <QColor>
#include <QDateTime>
#include <QHash>
#include <QPixmapCache>
#include <QPointer>
#include "core/UUID.h"
class Database;
class Group;

class Metadata final:public QObject
{
	Q_OBJECT public:
	explicit Metadata(
		QObject* parent = nullptr
	);

	struct MetadataData
	{
		QString generator;
		QString name;
		QDateTime nameChanged;
		QString description;
		QDateTime descriptionChanged;
		QString defaultUserName;
		QDateTime defaultUserNameChanged;
		int maintenanceHistoryDays;
		QColor color;
		bool recycleBinEnabled;
		int historyMaxItems;
		int historyMaxSize;
		int masterKeyChangeRec;
		int masterKeyChangeForce;
		bool protectTitle;
		bool protectUsername;
		bool protectPassword;
		bool protectUrl;
		bool protectNotes;
		// bool autoEnableVisualHiding;
	};

	QString getGenerator() const;
	QString getName() const;
	QDateTime getNameChangedTime() const;
	QString getDescription() const;
	QDateTime getDescriptionTimeChanged() const;
	QString getDefaultUserName() const;
	QDateTime getDefaultUserNameChanged() const;
	int getMaintenanceHistoryDays() const;
	QColor getColor() const;
	bool protectTitle() const;
	bool protectUsername() const;
	bool protectPassword() const;
	bool protectUrl() const;
	bool protectNotes() const;
	// bool autoEnableVisualHiding() const;
	QImage getCustomIcon(
		const UUID &uuid
	) const;
	QPixmap getCustomIconPixmap(
		const UUID &uuid
	) const;
	QPixmap getCustomIconScaledPixmap(
		const UUID &uuid
	) const;
	bool containsCustomIcon(
		const UUID &uuid
	) const;
	QHash<UUID, QImage> getCustomIcons() const;
	QList<UUID> getCustomIconsOrder() const;
	bool recycleBinEnabled() const;
	QHash<UUID, QPixmap> customIconsScaledPixmaps() const;
	Group* getRecycleBin();
	const Group* getRecycleBin() const;
	QDateTime getRecycleBinChangedTime() const;
	const Group* getEntryTemplatesGroup() const;
	QDateTime getEntryTemplatesGroupChangedTime() const;
	const Group* getLastSelectedGroup() const;
	const Group* getLastTopVisibleGroup() const;
	QDateTime getMasterKeyChanged() const;
	int masterKeyChangeRec() const;
	int masterKeyChangeForce() const;
	int getHistoryMaxItems() const;
	int getHistoryMaxSize() const;
	QHash<QString, QString> getCustomFields() const;
	static const int DefaultHistoryMaxItems;
	static const int DefaultHistoryMaxSize;
	void setGenerator(
		const QString &value
	);
	void setName(
		const QString &value
	);
	void setNameChanged(
		const QDateTime &value
	);
	void setDescription(
		const QString &value
	);
	void setDescriptionChanged(
		const QDateTime &value
	);
	void setDefaultUserName(
		const QString &value
	);
	void setDefaultUserNameChanged(
		const QDateTime &value
	);
	void setMaintenanceHistoryDays(
		int value
	);
	void setColor(
		const QColor &value
	);
	void setProtectTitle(
		bool value
	);
	void setProtectUsername(
		bool value
	);
	void setProtectPassword(
		bool value
	);
	void setProtectUrl(
		bool value
	);
	void setProtectNotes(
		bool value
	);
	// void setAutoEnableVisualHiding(bool value);
	void addCustomIcon(
		const UUID &uuid,
		const QImage &icon
	);
	void addCustomIconScaled(
		const UUID &uuid,
		const QImage &icon
	);
	void removeCustomIcon(
		const UUID &uuid
	);
	void copyCustomIcons(
		const QSet<UUID> &iconList,
		const Metadata* otherMetadata
	);
	void setRecycleBinEnabled(
		bool value
	);
	void setRecycleBin(
		Group* group
	);
	void setRecycleBinChanged(
		const QDateTime &value
	);
	void setEntryTemplatesGroup(
		Group* group
	);
	void setEntryTemplatesGroupChanged(
		const QDateTime &value
	);
	void setLastSelectedGroup(
		Group* group
	);
	void setLastTopVisibleGroup(
		Group* group
	);
	void setMasterKeyChanged(
		const QDateTime &value
	);
	void setMasterKeyChangeRec(
		int value
	);
	void setMasterKeyChangeForce(
		int value
	);
	void setHistoryMaxItems(
		int value
	);
	void setHistoryMaxSize(
		int value
	);
	void addCustomField(
		const QString &key,
		const QString &value
	);
	void removeCustomField(
		const QString &key
	);
	void setUpdateDatetime(
		bool value
	);
	/*
	* Copy all attributes from other except:
	* - Group pointers/uuids
	* - Master key changed date
	* - Custom icons
	* - Custom fields
	*/
	void copyAttributesFrom(
		const Metadata* other
	);
Q_SIGNALS:
	void sig_nameTextChanged();
	void sig_modified();
private:
	template<class P, class V> bool set(
		P &property,
		const V &value
	);
	template<class P, class V> bool set(
		P &property,
		const V &value,
		QDateTime &dateTime
	);
	MetadataData metadata;
	QHash<UUID, QImage> customIcons;
	mutable QHash<UUID, QPixmapCache::Key> customIconCacheKeys;
	mutable QHash<UUID, QPixmapCache::Key> customIconScaledCacheKeys;
	QList<UUID> customIconsOrder;
	QPointer<Group> recycleBin;
	QDateTime recycleBinChanged;
	QPointer<Group> entryTemplatesGroup;
	QDateTime entryTemplatesGroupChanged;
	QPointer<Group> lastSelectedGroup;
	QPointer<Group> lastTopVisibleGroup;
	QDateTime masterKeyChanged;
	QHash<QString, QString> customFields;
	bool updateDatetime;
};
#endif // KEEPASSX_METADATA_H
