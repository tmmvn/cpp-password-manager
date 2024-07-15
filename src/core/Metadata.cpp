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
#include "Metadata.h"
#include "core/Entry.h"
#include "core/Group.h"
#include "core/Tools.h"
const int Metadata::DefaultHistoryMaxItems = 10;
const int Metadata::DefaultHistoryMaxSize = 6 * 1024 * 1024;

Metadata::Metadata(
	QObject* parent
)
	: QObject(
		parent
	),
	updateDatetime(
		true
	)
{
	this->metadata.generator = "KeePass";
	this->metadata.maintenanceHistoryDays = 365;
	this->metadata.masterKeyChangeRec = -1;
	this->metadata.masterKeyChangeForce = -1;
	this->metadata.historyMaxItems = DefaultHistoryMaxItems;
	this->metadata.historyMaxSize = DefaultHistoryMaxSize;
	this->metadata.recycleBinEnabled = true;
	this->metadata.protectTitle = false;
	this->metadata.protectUsername = false;
	this->metadata.protectPassword = true;
	this->metadata.protectUrl = false;
	this->metadata.protectNotes = false;
	// m_data.autoEnableVisualHiding = false;
	const QDateTime now_ = QDateTime::currentDateTimeUtc();
	this->metadata.nameChanged = now_;
	this->metadata.descriptionChanged = now_;
	this->metadata.defaultUserNameChanged = now_;
	this->recycleBinChanged = now_;
	this->entryTemplatesGroupChanged = now_;
	this->masterKeyChanged = now_;
}

template<class P, class V> bool Metadata::set(
	P &property,
	const V &value
)
{
	if(property != value)
	{
		property = value;
		sig_modified();
		return true;
	}
	return false;
}

template<class P, class V> bool Metadata::set(
	P &property,
	const V &value,
	QDateTime &dateTime
)
{
	if(property != value)
	{
		property = value;
		if(this->updateDatetime)
		{
			dateTime = QDateTime::currentDateTimeUtc();
		}
		sig_modified();
		return true;
	}
	return false;
}

void Metadata::setUpdateDatetime(
	const bool value
)
{
	this->updateDatetime = value;
}

void Metadata::copyAttributesFrom(
	const Metadata* other
)
{
	this->metadata = other->metadata;
}

QString Metadata::getGenerator() const
{
	return this->metadata.generator;
}

QString Metadata::getName() const
{
	return this->metadata.name;
}

QDateTime Metadata::getNameChangedTime() const
{
	return this->metadata.nameChanged;
}

QString Metadata::getDescription() const
{
	return this->metadata.description;
}

QDateTime Metadata::getDescriptionTimeChanged() const
{
	return this->metadata.descriptionChanged;
}

QString Metadata::getDefaultUserName() const
{
	return this->metadata.defaultUserName;
}

QDateTime Metadata::getDefaultUserNameChanged() const
{
	return this->metadata.defaultUserNameChanged;
}

int Metadata::getMaintenanceHistoryDays() const
{
	return this->metadata.maintenanceHistoryDays;
}

QColor Metadata::getColor() const
{
	return this->metadata.color;
}

bool Metadata::protectTitle() const
{
	return this->metadata.protectTitle;
}

bool Metadata::protectUsername() const
{
	return this->metadata.protectUsername;
}

bool Metadata::protectPassword() const
{
	return this->metadata.protectPassword;
}

bool Metadata::protectUrl() const
{
	return this->metadata.protectUrl;
}

bool Metadata::protectNotes() const
{
	return this->metadata.protectNotes;
}

/*bool Metadata::autoEnableVisualHiding() const
{
    return m_autoEnableVisualHiding;
}*/
QImage Metadata::getCustomIcon(
	const UUID &uuid
) const
{
	return this->customIcons.value(
		uuid
	);
}

QPixmap Metadata::getCustomIconPixmap(
	const UUID &uuid
) const
{
	QPixmap pixmap_;
	if(!this->customIcons.contains(
		uuid
	))
	{
		return pixmap_;
	}
	if(QPixmapCache::Key &cacheKey_ = this->customIconCacheKeys[uuid];
		!QPixmapCache::find(
			cacheKey_,
			&pixmap_
		))
	{
		pixmap_ = QPixmap::fromImage(
			this->customIcons.value(
				uuid
			)
		);
		cacheKey_ = QPixmapCache::insert(
			pixmap_
		);
	}
	return pixmap_;
}

QPixmap Metadata::getCustomIconScaledPixmap(
	const UUID &uuid
) const
{
	QPixmap pixmap_;
	if(!this->customIcons.contains(
		uuid
	))
	{
		return pixmap_;
	}
	if(QPixmapCache::Key &cacheKey = this->customIconScaledCacheKeys[uuid];
		!QPixmapCache::find(
			cacheKey,
			&pixmap_
		))
	{
		const QImage image_ = this->customIcons.value(
			uuid
		).scaled(
			16,
			16,
			Qt::KeepAspectRatio,
			Qt::SmoothTransformation
		);
		pixmap_ = QPixmap::fromImage(
			image_
		);
		cacheKey = QPixmapCache::insert(
			pixmap_
		);
	}
	return pixmap_;
}

bool Metadata::containsCustomIcon(
	const UUID &uuid
) const
{
	return this->customIcons.contains(
		uuid
	);
}

QHash<UUID, QImage> Metadata::getCustomIcons() const
{
	return this->customIcons;
}

QHash<UUID, QPixmap> Metadata::customIconsScaledPixmaps() const
{
	QHash<UUID, QPixmap> result_;
	for(const UUID &uuid_: this->customIconsOrder)
	{
		result_.insert(
			uuid_,
			getCustomIconScaledPixmap(
				uuid_
			)
		);
	}
	return result_;
}

QList<UUID> Metadata::getCustomIconsOrder() const
{
	return this->customIconsOrder;
}

bool Metadata::recycleBinEnabled() const
{
	return this->metadata.recycleBinEnabled;
}

Group* Metadata::getRecycleBin()
{
	return this->recycleBin;
}

const Group* Metadata::getRecycleBin() const
{
	return this->recycleBin;
}

QDateTime Metadata::getRecycleBinChangedTime() const
{
	return this->recycleBinChanged;
}

const Group* Metadata::getEntryTemplatesGroup() const
{
	return this->entryTemplatesGroup;
}

QDateTime Metadata::getEntryTemplatesGroupChangedTime() const
{
	return this->entryTemplatesGroupChanged;
}

const Group* Metadata::getLastSelectedGroup() const
{
	return this->lastSelectedGroup;
}

const Group* Metadata::getLastTopVisibleGroup() const
{
	return this->lastTopVisibleGroup;
}

QDateTime Metadata::getMasterKeyChanged() const
{
	return this->masterKeyChanged;
}

int Metadata::masterKeyChangeRec() const
{
	return this->metadata.masterKeyChangeRec;
}

int Metadata::masterKeyChangeForce() const
{
	return this->metadata.masterKeyChangeForce;
}

int Metadata::getHistoryMaxItems() const
{
	return this->metadata.historyMaxItems;
}

int Metadata::getHistoryMaxSize() const
{
	return this->metadata.historyMaxSize;
}

QHash<QString, QString> Metadata::getCustomFields() const
{
	return this->customFields;
}

void Metadata::setGenerator(
	const QString &value
)
{
	this->set(
		this->metadata.generator,
		value
	);
}

void Metadata::setName(
	const QString &value
)
{
	if(this->set(
		this->metadata.name,
		value,
		this->metadata.nameChanged
	))
	{
		sig_nameTextChanged();
	}
}

void Metadata::setNameChanged(
	const QDateTime &value
)
{
	if(value.timeSpec() != Qt::UTC)
	{
		return;
	}
	this->metadata.nameChanged = value;
}

void Metadata::setDescription(
	const QString &value
)
{
	this->set(
		this->metadata.description,
		value,
		this->metadata.descriptionChanged
	);
}

void Metadata::setDescriptionChanged(
	const QDateTime &value
)
{
	if(value.timeSpec() != Qt::UTC)
	{
		return;
	}
	this->metadata.descriptionChanged = value;
}

void Metadata::setDefaultUserName(
	const QString &value
)
{
	this->set(
		this->metadata.defaultUserName,
		value,
		this->metadata.defaultUserNameChanged
	);
}

void Metadata::setDefaultUserNameChanged(
	const QDateTime &value
)
{
	if(value.timeSpec() != Qt::UTC)
	{
		return;
	}
	this->metadata.defaultUserNameChanged = value;
}

void Metadata::setMaintenanceHistoryDays(
	const int value
)
{
	this->set(
		this->metadata.maintenanceHistoryDays,
		value
	);
}

void Metadata::setColor(
	const QColor &value
)
{
	this->set(
		this->metadata.color,
		value
	);
}

void Metadata::setProtectTitle(
	const bool value
)
{
	this->set(
		this->metadata.protectTitle,
		value
	);
}

void Metadata::setProtectUsername(
	const bool value
)
{
	this->set(
		this->metadata.protectUsername,
		value
	);
}

void Metadata::setProtectPassword(
	const bool value
)
{
	this->set(
		this->metadata.protectPassword,
		value
	);
}

void Metadata::setProtectUrl(
	const bool value
)
{
	this->set(
		this->metadata.protectUrl,
		value
	);
}

void Metadata::setProtectNotes(
	const bool value
)
{
	this->set(
		this->metadata.protectNotes,
		value
	);
}

/*void Metadata::setAutoEnableVisualHiding(bool value)
{
    set(m_autoEnableVisualHiding, value);
}*/
void Metadata::addCustomIcon(
	const UUID &uuid,
	const QImage &icon
)
{
	if(uuid.isNull())
	{
		return;
	}
	if(this->customIcons.contains(
		uuid
	))
	{
		return;
	}
	this->customIcons.insert(
		uuid,
		icon
	);
	// reset cache in case there is also an icon with that uuid
	this->customIconCacheKeys[uuid] = QPixmapCache::Key();
	this->customIconScaledCacheKeys[uuid] = QPixmapCache::Key();
	this->customIconsOrder.append(
		uuid
	);
	if(this->customIcons.count() != this->customIconsOrder.count())
	{
		return;
	}
	sig_modified();
}

void Metadata::addCustomIconScaled(
	const UUID &uuid,
	const QImage &icon
)
{
	QImage iconScaled_;
	// scale down to 128x128 if icon is larger
	if(icon.width() > 128 || icon.height() > 128)
	{
		iconScaled_ = icon.scaled(
			QSize(
				128,
				128
			),
			Qt::KeepAspectRatio,
			Qt::SmoothTransformation
		);
	}
	else
	{
		iconScaled_ = icon;
	}
	this->addCustomIcon(
		uuid,
		iconScaled_
	);
}

void Metadata::removeCustomIcon(
	const UUID &uuid
)
{
	if(uuid.isNull())
	{
		return;
	}
	if(!this->customIcons.contains(
		uuid
	))
	{
		return;
	}
	this->customIcons.remove(
		uuid
	);
	QPixmapCache::remove(
		this->customIconCacheKeys.value(
			uuid
		)
	);
	this->customIconCacheKeys.remove(
		uuid
	);
	QPixmapCache::remove(
		this->customIconScaledCacheKeys.value(
			uuid
		)
	);
	this->customIconScaledCacheKeys.remove(
		uuid
	);
	this->customIconsOrder.removeAll(
		uuid
	);
	if(this->customIcons.count() != this->customIconsOrder.count())
	{
		return;
	}
	sig_modified();
}

void Metadata::copyCustomIcons(
	const QSet<UUID> &iconList,
	const Metadata* otherMetadata
)
{
	for(const UUID &uuid_: iconList)
	{
		if(!otherMetadata->containsCustomIcon(
			uuid_
		))
		{
			continue;
		}
		if(!this->containsCustomIcon(
			uuid_
		) && otherMetadata->containsCustomIcon(
			uuid_
		))
		{
			this->addCustomIcon(
				uuid_,
				otherMetadata->getCustomIcon(
					uuid_
				)
			);
		}
	}
}

void Metadata::setRecycleBinEnabled(
	const bool value
)
{
	this->set(
		this->metadata.recycleBinEnabled,
		value
	);
}

void Metadata::setRecycleBin(
	Group* group
)
{
	this->set(
		this->recycleBin,
		group,
		this->recycleBinChanged
	);
}

void Metadata::setRecycleBinChanged(
	const QDateTime &value
)
{
	if(value.timeSpec() != Qt::UTC)
	{
		return;
	};
	this->recycleBinChanged = value;
}

void Metadata::setEntryTemplatesGroup(
	Group* group
)
{
	this->set(
		this->entryTemplatesGroup,
		group,
		this->entryTemplatesGroupChanged
	);
}

void Metadata::setEntryTemplatesGroupChanged(
	const QDateTime &value
)
{
	if(value.timeSpec() != Qt::UTC)
	{
		return;
	};
	this->entryTemplatesGroupChanged = value;
}

void Metadata::setLastSelectedGroup(
	Group* group
)
{
	this->set(
		this->lastSelectedGroup,
		group
	);
}

void Metadata::setLastTopVisibleGroup(
	Group* group
)
{
	this->set(
		this->lastTopVisibleGroup,
		group
	);
}

void Metadata::setMasterKeyChanged(
	const QDateTime &value
)
{
	if(value.timeSpec() != Qt::UTC)
	{
		return;
	};
	this->masterKeyChanged = value;
}

void Metadata::setMasterKeyChangeRec(
	const int value
)
{
	this->set(
		this->metadata.masterKeyChangeRec,
		value
	);
}

void Metadata::setMasterKeyChangeForce(
	const int value
)
{
	this->set(
		this->metadata.masterKeyChangeForce,
		value
	);
}

void Metadata::setHistoryMaxItems(
	const int value
)
{
	this->set(
		this->metadata.historyMaxItems,
		value
	);
}

void Metadata::setHistoryMaxSize(
	const int value
)
{
	this->set(
		this->metadata.historyMaxSize,
		value
	);
}

void Metadata::addCustomField(
	const QString &key,
	const QString &value
)
{
	if(this->customFields.contains(
		key
	))
	{
		return;
	};
	this->customFields.insert(
		key,
		value
	);
	sig_modified();
}

void Metadata::removeCustomField(
	const QString &key
)
{
	if(!this->customFields.contains(
		key
	))
	{
		return;
	};
	this->customFields.remove(
		key
	);
	sig_modified();
}
