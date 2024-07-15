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
#include "Entry.h"
#include "core/Database.h"
#include "core/DatabaseIcons.h"
#include "core/Group.h"
#include "core/Metadata.h"
const int Entry::DefaultIconNumber = 0;

Entry::Entry()
	: attributes(
		new EntryAttributes(
			this
		)
	),
	attachments(
		new EntryAttachments(
			this
		)
	),
	tmpHistoryItem(
		nullptr
	),
	modifiedSinceBegin(
		false
	),
	updateTimeinfo(
		true
	)
{
	this->data.iconNumber = this->DefaultIconNumber;
	this->data.autoTypeEnabled = true;
	this->data.autoTypeObfuscation = 0;
	this->connect(
		this->attributes,
		&EntryAttributes::sig_modified,
		this,
		&Entry::sig_modified
	);
	this->connect(
		this->attributes,
		&EntryAttributes::sig_defaultKeyModified,
		this,
		&Entry::do_emitDataChanged
	);
	this->connect(
		this->attachments,
		&EntryAttachments::sig_modified,
		this,
		&Entry::sig_modified
	);
	this->connect(
		this,
		&Entry::sig_modified,
		this,
		&Entry::do_updateTimeinfo
	);
	this->connect(
		this,
		&Entry::sig_modified,
		this,
		&Entry::do_updateModifiedSinceBegin
	);
}

Entry::~Entry()
{
	if(this->group)
	{
		this->group->removeEntry(
			this
		);
		if(this->group->getDatabase())
		{
			this->group->getDatabase()->addDeletedObject(
				uuid
			);
		}
	}
	qDeleteAll(
		this->history
	);
}

template<class T> inline bool Entry::set(
	T &property,
	const T &value
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

void Entry::do_updateTimeinfo()
{
	if(this->updateTimeinfo)
	{
		this->data.timeInfo.setLastModificationTime(
			QDateTime::currentDateTimeUtc()
		);
		this->data.timeInfo.setLastAccessTime(
			QDateTime::currentDateTimeUtc()
		);
	}
}

void Entry::setUpdateTimeinfo(
	const bool value
)
{
	this->updateTimeinfo = value;
}

UUID Entry::getUUID() const
{
	return this->uuid;
}

QImage Entry::getIcon() const
{
	if(this->data.customIcon.isNull())
	{
		return DatabaseIcons::getInstance()->icon(
			this->data.iconNumber
		);
	}
	if(this->getDatabase() == nullptr)
	{
		return QImage();
	};
	if(this->getDatabase())
	{
		return this->getDatabase()->getMetadata()->getCustomIcon(
			data.customIcon
		);
	}
	return QImage();
}

QPixmap Entry::getIconPixmap() const
{
	if(this->data.customIcon.isNull())
	{
		return DatabaseIcons::getInstance()->iconPixmap(
			this->data.iconNumber
		);
	}
	if(this->getDatabase() == nullptr)
	{
		return QPixmap();
	};
	if(this->getDatabase())
	{
		return this->getDatabase()->getMetadata()->getCustomIconPixmap(
			this->data.customIcon
		);
	}
	return QPixmap();
}

QPixmap Entry::getIconScaledPixmap() const
{
	if(this->data.customIcon.isNull())
	{
		// built-in icons are 16x16 so don't need to be scaled
		return DatabaseIcons::getInstance()->iconPixmap(
			this->data.iconNumber
		);
	}
	if(this->getDatabase() == nullptr)
	{
		return QPixmap();
	}
	return this->getDatabase()->getMetadata()->getCustomIconScaledPixmap(
		this->data.customIcon
	);
}

int Entry::getIconNumber() const
{
	return this->data.iconNumber;
}

UUID Entry::getIconUUID() const
{
	return this->data.customIcon;
}

QColor Entry::getForegroundColor() const
{
	return this->data.foregroundColor;
}

QColor Entry::getBackgroundColor() const
{
	return this->data.backgroundColor;
}

QString Entry::getOverrideURL() const
{
	return this->data.overrideUrl;
}

QString Entry::getTags() const
{
	return this->data.tags;
}

TimeInfo Entry::getTimeInfo() const
{
	return this->data.timeInfo;
}

bool Entry::isAutoTypeEnabled() const
{
	return this->data.autoTypeEnabled;
}

int Entry::getAutoTypeObfuscation() const
{
	return this->data.autoTypeObfuscation;
}

QString Entry::defaultAutoTypeSequence() const
{
	return this->data.defaultAutoTypeSequence;
}

QString Entry::getTitle() const
{
	return this->attributes->getValue(
		EntryAttributes::TitleKey
	);
}

QString Entry::getURL() const
{
	return this->attributes->getValue(
		EntryAttributes::URLKey
	);
}

QString Entry::getUsername() const
{
	return this->attributes->getValue(
		EntryAttributes::UserNameKey
	);
}

QString Entry::getPassword() const
{
	return this->attributes->getValue(
		EntryAttributes::PasswordKey
	);
}

QString Entry::getNotes() const
{
	return this->attributes->getValue(
		EntryAttributes::NotesKey
	);
}

bool Entry::isExpired() const
{
	return this->data.timeInfo.getExpires() && this->data.timeInfo.
		getExpiryTime() < QDateTime::currentDateTimeUtc();
}

EntryAttributes* Entry::getAttributes()
{
	return this->attributes;
}

const EntryAttributes* Entry::getAttributes() const
{
	return this->attributes;
}

EntryAttachments* Entry::getAttachments()
{
	return this->attachments;
}

const EntryAttachments* Entry::getAttachments() const
{
	return this->attachments;
}

void Entry::setUUID(
	const UUID &uuid
)
{
	if(uuid.isNull())
	{
		return;
	};
	this->set(
		this->uuid,
		uuid
	);
}

void Entry::setIcon(
	const int iconNumber
)
{
	if(iconNumber < 0)
	{
		return;
	};
	if(this->data.iconNumber != iconNumber || !this->data.customIcon.isNull())
	{
		this->data.iconNumber = iconNumber;
		this->data.customIcon = UUID();
		 sig_modified();
		this->do_emitDataChanged();
	}
}

void Entry::setIcon(
	const UUID &uuid
)
{
	if(uuid.isNull())
	{
		return;
	};
	if(this->data.customIcon != uuid)
	{
		this->data.customIcon = uuid;
		this->data.iconNumber = 0;
		 sig_modified();
		this->do_emitDataChanged();
	}
}

void Entry::setForegroundColor(
	const QColor &color
)
{
	this->set(
		this->data.foregroundColor,
		color
	);
}

void Entry::setBackgroundColor(
	const QColor &color
)
{
	this->set(
		this->data.backgroundColor,
		color
	);
}

void Entry::setOverrideURL(
	const QString &url
)
{
	this->set(
		this->data.overrideUrl,
		url
	);
}

void Entry::setTags(
	const QString &tags
)
{
	this->set(
		this->data.tags,
		tags
	);
}

void Entry::setTimeInfo(
	const TimeInfo &timeInfo
)
{
	this->data.timeInfo = timeInfo;
}

void Entry::setAutoTypeEnabled(
	const bool enable
)
{
	this->set(
		this->data.autoTypeEnabled,
		enable
	);
}

void Entry::setAutoTypeObfuscation(
	const int obfuscation
)
{
	this->set(
		this->data.autoTypeObfuscation,
		obfuscation
	);
}

void Entry::setDefaultAutoTypeSequence(
	const QString &sequence
)
{
	this->set(
		this->data.defaultAutoTypeSequence,
		sequence
	);
}

void Entry::setTitle(
	const QString &title
) const
{
	this->attributes->set(
		EntryAttributes::TitleKey,
		title,
		this->attributes->isProtected(
			EntryAttributes::TitleKey
		)
	);
}

void Entry::setURL(
	const QString &url
) const
{
	this->attributes->set(
		EntryAttributes::URLKey,
		url,
		this->attributes->isProtected(
			EntryAttributes::URLKey
		)
	);
}

void Entry::setUsername(
	const QString &username
) const
{
	this->attributes->set(
		EntryAttributes::UserNameKey,
		username,
		this->attributes->isProtected(
			EntryAttributes::UserNameKey
		)
	);
}

void Entry::setPassword(
	const QString &password
) const
{
	this->attributes->set(
		EntryAttributes::PasswordKey,
		password,
		this->attributes->isProtected(
			EntryAttributes::PasswordKey
		)
	);
}

void Entry::setNotes(
	const QString &notes
) const
{
	this->attributes->set(
		EntryAttributes::NotesKey,
		notes,
		this->attributes->isProtected(
			EntryAttributes::NotesKey
		)
	);
}

void Entry::setExpires(
	const bool &value
)
{
	if(this->data.timeInfo.getExpires() != value)
	{
		this->data.timeInfo.setExpires(
			value
		);
		 sig_modified();
	}
}

void Entry::setExpiryTime(
	const QDateTime &dateTime
)
{
	if(this->data.timeInfo.getExpiryTime() != dateTime)
	{
		this->data.timeInfo.setExpiryTime(
			dateTime
		);
		 sig_modified();
	}
}

QList<Entry*> Entry::getHistoryItems()
{
	return this->history;
}

const QList<Entry*> &Entry::getHistoryItems() const
{
	return this->history;
}

void Entry::addHistoryItem(
	Entry* entry
)
{
	if(entry->parent())
	{
		return;
	};
	this->history.append(
		entry
	);
	 sig_modified();
}

void Entry::removeHistoryItems(
	const QList<Entry*> &historyEntries
)
{
	if(historyEntries.isEmpty())
	{
		return;
	}
	for(Entry* entry_: historyEntries)
	{
		if(entry_->parent())
		{
			return;
		};
		if(entry_->getUUID() != getUUID())
		{
			return;
		}
		if(!this->history.contains(
			entry_
		))
		{
			return;
		}
		this->history.removeOne(
			entry_
		);
		delete entry_;
	}
	 sig_modified();
}

void Entry::truncateHistory()
{
	const Database* db_ = this->getDatabase();
	if(!db_)
	{
		return;
	}
	if(const int histMaxItems_ = db_->getMetadata()->getHistoryMaxItems();
		histMaxItems_ > -1)
	{
		auto historyCount_ = 0;
		QMutableListIterator i_(
			history
		);
		i_.toBack();
		while(i_.hasPrevious())
		{
			historyCount_++;
			const Entry* entry_ = i_.previous();
			if(historyCount_ > histMaxItems_)
			{
				delete entry_;
				i_.remove();
			}
		}
	}
	if(const int histMaxSize_ = db_->getMetadata()->getHistoryMaxSize();
		histMaxSize_ > -1)
	{
		auto size_ = 0;
		auto foundAttachements_ = QSet(
			this->getAttachments()->getValues().constBegin(),
			this->getAttachments()->getValues().constEnd()
		);
		QMutableListIterator i_(
			this->history
		);
		i_.toBack();
		while(i_.hasPrevious())
		{
			Entry* historyItem_ = i_.previous();
			// don't calculate size if it's already above the maximum
			if(size_ <= histMaxSize_)
			{
				size_ += historyItem_->getAttributes()->getAttributesSize();
				const QSet<QByteArray> newAttachments_ = QSet(
					historyItem_->getAttachments()->getValues().constBegin(),
					historyItem_->getAttachments()->getValues().constEnd()
				) - foundAttachements_;
				for(const QByteArray &attachment: newAttachments_)
				{
					size_ += static_cast<int>(attachment.size());
				}
				foundAttachements_ += newAttachments_;
			}
			if(size_ > histMaxSize_)
			{
				delete historyItem_;
				i_.remove();
			}
		}
	}
}

Entry* Entry::clone(
	const CloneFlags flags
) const
{
	const auto entry_ = new Entry();
	entry_->setUpdateTimeinfo(
		false
	);
	if(flags & this->CloneNewUuid)
	{
		entry_->uuid = UUID::random();
	}
	else
	{
		entry_->uuid = uuid;
	}
	entry_->data = this->data;
	entry_->attributes->copyDataFrom(
		this->attributes
	);
	entry_->attachments->copyDataFrom(
		this->attachments
	);
	if(flags & this->CloneIncludeHistory)
	{
		for(const Entry* historyItem_: history)
		{
			Entry* historyItemClone_ = historyItem_->clone(
				flags & ~this->CloneIncludeHistory & ~this->CloneNewUuid
			);
			historyItemClone_->setUpdateTimeinfo(
				false
			);
			historyItemClone_->setUUID(
				entry_->getUUID()
			);
			historyItemClone_->setUpdateTimeinfo(
				true
			);
			entry_->addHistoryItem(
				historyItemClone_
			);
		}
	}
	entry_->setUpdateTimeinfo(
		true
	);
	if(flags & this->CloneResetTimeInfo)
	{
		const QDateTime now_ = QDateTime::currentDateTimeUtc();
		entry_->data.timeInfo.setCreationTime(
			now_
		);
		entry_->data.timeInfo.setLastModificationTime(
			now_
		);
		entry_->data.timeInfo.setLastAccessTime(
			now_
		);
		entry_->data.timeInfo.setLocationChanged(
			now_
		);
	}
	return entry_;
}

void Entry::copyDataFrom(
	const Entry* other
)
{
	this->setUpdateTimeinfo(
		false
	);
	this->data = other->data;
	this->attributes->copyDataFrom(
		other->attributes
	);
	this->attachments->copyDataFrom(
		other->attachments
	);
	this->setUpdateTimeinfo(
		true
	);
}

void Entry::beginUpdate()
{
	if(this->tmpHistoryItem)
	{
		return;
	};
	this->tmpHistoryItem = new Entry();
	this->tmpHistoryItem->setUpdateTimeinfo(
		false
	);
	this->tmpHistoryItem->uuid = this->uuid;
	this->tmpHistoryItem->data = this->data;
	this->tmpHistoryItem->attributes->copyDataFrom(
		this->attributes
	);
	this->tmpHistoryItem->attachments->copyDataFrom(
		this->attachments
	);
	this->modifiedSinceBegin = false;
}

bool Entry::endUpdate()
{
	if(!this->tmpHistoryItem)
	{
		return false;
	};
	if(this->modifiedSinceBegin)
	{
		this->tmpHistoryItem->setUpdateTimeinfo(
			true
		);
		this->addHistoryItem(
			this->tmpHistoryItem
		);
		this->truncateHistory();
	}
	else
	{
		delete this->tmpHistoryItem;
	}
	this->tmpHistoryItem = nullptr;
	return this->modifiedSinceBegin;
}

void Entry::do_updateModifiedSinceBegin()
{
	this->modifiedSinceBegin = true;
}

Group* Entry::getGroup()
{
	return this->group;
}

const Group* Entry::getGroup() const
{
	return this->group;
}

void Entry::setGroup(
	Group* group
)
{
	if(!group)
	{
		return;
	}
	if(this->group == group)
	{
		return;
	}
	if(this->group)
	{
		this->group->removeEntry(
			this
		);
		if(this->group->getDatabase() && this->group->getDatabase() != group->
			getDatabase())
		{
			this->group->getDatabase()->addDeletedObject(
				uuid
			);
			// copy custom icon to the new database
			if(!getIconUUID().isNull() && group->getDatabase() && this->group->
				getDatabase()->getMetadata()->containsCustomIcon(
					this->getIconUUID()
				) && !group->getDatabase()->getMetadata()->containsCustomIcon(
					this->getIconUUID()
				))
			{
				group->getDatabase()->getMetadata()->addCustomIcon(
					this->getIconUUID(),
					this->getIcon()
				);
			}
		}
	}
	this->group = group;
	group->addEntry(
		this
	);
	setParent(
		group
	);
	if(this->updateTimeinfo)
	{
		this->data.timeInfo.setLocationChanged(
			QDateTime::currentDateTimeUtc()
		);
	}
}

void Entry::do_emitDataChanged()
{
	 sig_dataChanged(
		this
	);
}

const Database* Entry::getDatabase() const
{
	if(this->group)
	{
		return this->group->getDatabase();
	}
	else
	{
		return nullptr;
	}
}

QString Entry::resolvePlaceholders(
	const QString &str
) const
{
	QString result_ = str;
	result_.replace(
		"{TITLE}",
		this->getTitle(),
		Qt::CaseInsensitive
	);
	result_.replace(
		"{USERNAME}",
		this->getUsername(),
		Qt::CaseInsensitive
	);
	result_.replace(
		"{URL}",
		this->getURL(),
		Qt::CaseInsensitive
	);
	result_.replace(
		"{PASSWORD}",
		this->getPassword(),
		Qt::CaseInsensitive
	);
	result_.replace(
		"{NOTES}",
		this->getNotes(),
		Qt::CaseInsensitive
	);
	// TODO: lots of other placeholders missing
	return result_;
}
