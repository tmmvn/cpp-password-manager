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
#include "Group.h"
#include "core/Config.h"
#include "core/Global.h"
#include "core/DatabaseIcons.h"
#include "core/Metadata.h"
const int Group::DefaultIconNumber = 48;
const int Group::RecycleBinIconNumber = 43;

Group::Group()
	: updateTimeinfo(
		true
	)
{
	this->data.iconNumber = this->DefaultIconNumber;
	this->data.isExpanded = true;
	this->data.searchingEnabled = this->Inherit;
}

Group::~Group()
{
	// Destroy entries and children manually so DeletedObjects can be added
	// to database.
	const QList<Entry*> entries_ = this->entries;
	for(const Entry* entry_: entries_)
	{
		delete entry_;
	}
	const QList<Group*> children_ = this->children;
	for(const Group* group_: children_)
	{
		delete group_;
	}
	if(this->db && this->parent)
	{
		DeletedObject delGroup_;
		delGroup_.deletionTime = QDateTime::currentDateTimeUtc();
		delGroup_.uuid = uuid;
		this->db->addDeletedObject(
			delGroup_
		);
	}
	this->cleanupParent();
}

Group* Group::createRecycleBin()
{
	const auto recycleBin_ = new Group();
	recycleBin_->setUuid(
		UUID::random()
	);
	recycleBin_->setName(
		tr(
			"Recycle Bin"
		)
	);
	recycleBin_->setIcon(
		RecycleBinIconNumber
	);
	recycleBin_->setSearchingEnabled(
		Disable
	);
	return recycleBin_;
}

template<class P, class V> bool Group::set(
	P &property,
	const V &value
)
{
	if(property != value)
	{
		property = value;
		this->getUpdateTimeinfo();
		sig_modified();
		return true;
	}
	return false;
}

void Group::getUpdateTimeinfo()
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

void Group::setUpdateTimeinfo(
	const bool value
)
{
	this->updateTimeinfo = value;
}

UUID Group::getUUID() const
{
	return this->uuid;
}

QString Group::getName() const
{
	return this->data.name;
}

QString Group::getNotes() const
{
	return this->data.notes;
}

QImage Group::getIcon() const
{
	if(this->data.customIcon.isNull())
	{
		return DatabaseIcons::getInstance()->icon(
			this->data.iconNumber
		);
	}
	if(this->db == nullptr)
	{
		return QImage();
	}
	if(this->db)
	{
		return this->db->getMetadata()->getCustomIcon(
			this->data.customIcon
		);
	}
	return QImage();
}

QPixmap Group::getIconPixmap() const
{
	if(this->data.customIcon.isNull())
	{
		return DatabaseIcons::getInstance()->iconPixmap(
			this->data.iconNumber
		);
	}
	if(this->db == nullptr)
	{
		return QPixmap();
	}
	if(this->db)
	{
		return this->db->getMetadata()->getCustomIconPixmap(
			this->data.customIcon
		);
	}
	return QPixmap();
}

QPixmap Group::getIconScaledPixmap() const
{
	if(this->data.customIcon.isNull())
	{
		// built-in icons are 16x16 so don't need to be scaled
		return DatabaseIcons::getInstance()->iconPixmap(
			this->data.iconNumber
		);
	}
	if(this->db == nullptr)
	{
		return QPixmap();
	}
	if(this->db)
	{
		return this->db->getMetadata()->getCustomIconScaledPixmap(
			this->data.customIcon
		);
	}
	return QPixmap();
}

int Group::getIconNumber() const
{
	return this->data.iconNumber;
}

UUID Group::getIconUUID() const
{
	return this->data.customIcon;
}

TimeInfo Group::getTimeInfo() const
{
	return this->data.timeInfo;
}

bool Group::isExpanded() const
{
	return this->data.isExpanded;
}

Group::TriState Group::isSearchingEnabled() const
{
	return this->data.searchingEnabled;
}

Entry* Group::getLastTopVisibleEntry() const
{
	return this->lastTopVisibleEntry;
}

bool Group::isExpired() const
{
	return this->data.timeInfo.getExpires() && this->data.timeInfo.
		getExpiryTime() < QDateTime::currentDateTimeUtc();
}

void Group::setUuid(
	const UUID &uuid
)
{
	this->set(
		this->uuid,
		uuid
	);
}

void Group::setName(
	const QString &name
)
{
	if(this->set(
		this->data.name,
		name
	))
	{
		sig_dataChanged(
			this
		);
	}
}

void Group::setNotes(
	const QString &notes
)
{
	this->set(
		this->data.notes,
		notes
	);
}

void Group::setIcon(
	const int iconNumber
)
{
	if(iconNumber <= 0)
	{
		return;
	}
	if(this->data.iconNumber != iconNumber || !this->data.customIcon.isNull())
	{
		this->data.iconNumber = iconNumber;
		this->data.customIcon = UUID();
		this->getUpdateTimeinfo();
		sig_modified();
		sig_dataChanged(
			this
		);
	}
}

void Group::setIcon(
	const UUID &uuid
)
{
	if(uuid.isNull())
	{
		return;
	}
	if(this->data.customIcon != uuid)
	{
		this->data.customIcon = uuid;
		this->data.iconNumber = 0;
		this->getUpdateTimeinfo();
		sig_modified();
		sig_dataChanged(
			this
		);
	}
}

void Group::setTimeInfo(
	const TimeInfo &timeInfo
)
{
	this->data.timeInfo = timeInfo;
}

void Group::setExpanded(
	const bool expanded
)
{
	if(this->data.isExpanded != expanded)
	{
		this->data.isExpanded = expanded;
		this->getUpdateTimeinfo();
		sig_modified();
	}
}

void Group::setSearchingEnabled(
	const TriState enable
)
{
	this->set(
		this->data.searchingEnabled,
		enable
	);
}

void Group::setLastTopVisibleEntry(
	Entry* entry
)
{
	this->set(
		this->lastTopVisibleEntry,
		entry
	);
}

void Group::setExpires(
	const bool value
)
{
	if(this->data.timeInfo.getExpires() != value)
	{
		this->data.timeInfo.setExpires(
			value
		);
		this->getUpdateTimeinfo();
		sig_modified();
	}
}

void Group::setExpiryTime(
	const QDateTime &dateTime
)
{
	if(this->data.timeInfo.getExpiryTime() != dateTime)
	{
		this->data.timeInfo.setExpiryTime(
			dateTime
		);
		this->getUpdateTimeinfo();
		sig_modified();
	}
}

Group* Group::getParentGroup()
{
	return this->parent;
}

const Group* Group::getParentGroup() const
{
	return this->parent;
}

void Group::setParent(
	Group* parent,
	int index
)
{
	if(parent == nullptr)
	{
		return;
	}
	if(!(index >= -1 && index <= parent->getChildren().size()))
	{
		qWarning() << "Invalid index for adding a group to a parent group.";
		return; // Or return an error code
	}
	if(this->db && this->db->getRootGroup() == this)
	{
		qWarning() << "Cannot set a new parent for a root group.";
		return; // Or return an error code
	}
	const bool moveWithinDatabase_ = this->db && this->db == parent->db;
	if(index == -1)
	{
		index = static_cast<int>(parent->getChildren().size());
		if(this->getParentGroup() == parent)
		{
			index--;
		}
	}
	if(this->parent == parent && parent->getChildren().indexOf(
		this
	) == index)
	{
		return;
	}
	if(!moveWithinDatabase_)
	{
		this->cleanupParent();
		this->parent = parent;
		if(this->db)
		{
			this->recCreateDelObjects();
			// copy custom icon to the new database
			if(!this->getIconUUID().isNull() && this->parent->db && this->db->
				getMetadata()->containsCustomIcon(
					this->getIconUUID()
				) && !this->parent->db->getMetadata()->containsCustomIcon(
					this->getIconUUID()
				))
			{
				this->parent->db->getMetadata()->addCustomIcon(
					this->getIconUUID(),
					this->getIcon()
				);
			}
		}
		if(this->db != this->parent->db)
		{
			this->recSetDatabase(
				this->parent->db
			);
		}
		QObject::setParent(
			this->parent
		);
		sig_aboutToAdd(
			this,
			index
		);
		if(index > this->parent->children.size())
		{
			return;
		};
		this->parent->children.insert(
			index,
			this
		);
	}
	else
	{
		sig_aboutToMove(
			this,
			parent,
			index
		);
		parent->children.removeAll(
			this
		);
		this->parent = parent;
		QObject::setParent(
			this->parent
		);
		if(index > this->parent->children.size())
		{
			return;
		}
		this->parent->children.insert(
			index,
			this
		);
	}
	if(this->updateTimeinfo)
	{
		this->data.timeInfo.setLocationChanged(
			QDateTime::currentDateTimeUtc()
		);
	}
	sig_modified();
	if(!moveWithinDatabase_)
	{
		sig_added();
	}
	else
	{
		sig_moved();
	}
}

void Group::setParent(
	Database* db
)
{
	if(db == nullptr)
	{
		return;
	};
	if(db->getRootGroup() != this)
	{
		return;
	}
	this->cleanupParent();
	this->parent = nullptr;
	this->recSetDatabase(
		db
	);
	QObject::setParent(
		db
	);
}

Database* Group::getDatabase()
{
	return this->db;
}

const Database* Group::getDatabase() const
{
	return this->db;
}

QList<Group*> Group::getChildren()
{
	return this->children;
}

const QList<Group*> &Group::getChildren() const
{
	return this->children;
}

QList<Entry*> Group::getEntries()
{
	return this->entries;
}

const QList<Entry*> &Group::getEntries() const
{
	return this->entries;
}

QList<Entry*> Group::getEntriesRecursive(
	const bool includeHistoryItems
) const
{
	QList<Entry*> entryList_;
	entryList_.append(
		entries
	);
	if(includeHistoryItems)
	{
		for(Entry* entry_: this->entries)
		{
			entryList_.append(
				entry_->getHistoryItems()
			);
		}
	}
	for(const Group* group_: this->children)
	{
		entryList_.append(
			group_->getEntriesRecursive(
				includeHistoryItems
			)
		);
	}
	return entryList_;
}

QList<const Group*> Group::getGroupsRecursive(
	const bool includeSelf
) const
{
	QList<const Group*> groupList_;
	if(includeSelf)
	{
		groupList_.append(
			this
		);
	}
	for(const Group* group_: children)
	{
		groupList_.append(
			group_->getGroupsRecursive(
				true
			)
		);
	}
	return groupList_;
}

QList<Group*> Group::getGroupsRecursive(
	const bool includeSelf
)
{
	QList<Group*> groupList_;
	if(includeSelf)
	{
		groupList_.append(
			this
		);
	}
	for(Group* group_: asConst(
			children
		))
	{
		groupList_.append(
			group_->getGroupsRecursive(
				true
			)
		);
	}
	return groupList_;
}

QSet<UUID> Group::getCustomIconsRecursive() const
{
	QSet<UUID> result_;
	if(!this->getIconUUID().isNull())
	{
		result_.insert(
			this->getIconUUID()
		);
	}
	const QList<Entry*> entryList_ = this->getEntriesRecursive(
		true
	);
	for(const Entry* entry_: entryList_)
	{
		if(!entry_->getIconUUID().isNull())
		{
			result_.insert(
				entry_->getIconUUID()
			);
		}
	}
	for(const Group* group_: children)
	{
		result_.unite(
			group_->getCustomIconsRecursive()
		);
	}
	return result_;
}

Group* Group::clone(
	const Entry::CloneFlags entryFlags
) const
{
	const auto clonedGroup_ = new Group();
	clonedGroup_->setUpdateTimeinfo(
		false
	);
	clonedGroup_->setUuid(
		UUID::random()
	);
	clonedGroup_->data = this->data;
	const QList<Entry*> entryList_ = this->getEntries();
	for(const Entry* entry_: entryList_)
	{
		Entry* clonedEntry = entry_->clone(
			entryFlags
		);
		clonedEntry->setGroup(
			clonedGroup_
		);
	}
	const QList<Group*> childrenGroups_ = this->getChildren();
	for(const Group* groupChild_: childrenGroups_)
	{
		Group* clonedGroupChild = groupChild_->clone(
			entryFlags
		);
		clonedGroupChild->setParent(
			clonedGroup_
		);
	}
	clonedGroup_->setUpdateTimeinfo(
		true
	);
	const QDateTime now_ = QDateTime::currentDateTimeUtc();
	clonedGroup_->data.timeInfo.setCreationTime(
		now_
	);
	clonedGroup_->data.timeInfo.setLastModificationTime(
		now_
	);
	clonedGroup_->data.timeInfo.setLastAccessTime(
		now_
	);
	clonedGroup_->data.timeInfo.setLocationChanged(
		now_
	);
	return clonedGroup_;
}

void Group::copyDataFrom(
	const Group* other
)
{
	this->data = other->data;
	this->lastTopVisibleEntry = other->lastTopVisibleEntry;
}

void Group::addEntry(
	Entry* entry
)
{
	if(entry == nullptr)
	{
		return;
	};
	if(this->entries.contains(
		entry
	))
	{
		return;
	}
	sig_entryAboutToAdd(
		entry
	);
	this->entries << entry;
	this->connect(
		entry,
		&Entry::sig_dataChanged,
		this,
		&Group::sig_entryDataChanged
	);
	if(this->db)
	{
		this->connect(
			entry,
			&Entry::sig_modified,
			this->db,
			&Database::sig_modifiedImmediate
		);
	}
	sig_modified();
	sig_entryAdded(
		entry
	);
}

void Group::removeEntry(
	Entry* entry
)
{
	if(!this->entries.contains(
		entry
	))
	{
		return;
	}
	sig_entryAboutToRemove(
		entry
	);
	entry->disconnect(
		this
	);
	if(this->db)
	{
		entry->disconnect(
			this->db
		);
	}
	this->entries.removeAll(
		entry
	);
	sig_modified();
	sig_entryRemoved(
		entry
	);
}

void Group::recSetDatabase(
	Database* db
)
{
	if(db)
	{
		this->disconnect(
			this,
			&Group::sig_dataChanged,
			db,
			nullptr
		);
		this->disconnect(
			this,
			&Group::sig_aboutToRemove,
			db,
			nullptr
		);
		this->disconnect(
			this,
			&Group::sig_removed,
			db,
			nullptr
		);
		this->disconnect(
			this,
			&Group::sig_aboutToAdd,
			db,
			nullptr
		);
		this->disconnect(
			this,
			&Group::sig_added,
			db,
			nullptr
		);
		this->disconnect(
			this,
			&Group::sig_aboutToMove,
			db,
			nullptr
		);
		this->disconnect(
			this,
			&Group::sig_moved,
			db,
			nullptr
		);
		this->disconnect(
			this,
			&Group::sig_modified,
			db,
			nullptr
		);
	}
	for(const Entry* entry_: asConst(
			this->entries
		))
	{
		if(this->db)
		{
			entry_->disconnect(
				this->db
			);
		}
		if(db)
		{
			this->connect(
				entry_,
				&Entry::sig_modified,
				db,
				&Database::sig_modifiedImmediate
			);
		}
	}
	if(db)
	{
		this->connect(
			this,
			&Group::sig_dataChanged,
			db,
			&Database::sig_groupDataChanged
		);
		this->connect(
			this,
			&Group::sig_aboutToRemove,
			db,
			&Database::sig_groupAboutToRemove
		);
		this->connect(
			this,
			&Group::sig_removed,
			db,
			&Database::sig_groupRemoved
		);
		this->connect(
			this,
			&Group::sig_aboutToAdd,
			db,
			&Database::sig_groupAboutToAdd
		);
		this->connect(
			this,
			&Group::sig_added,
			db,
			&Database::sig_groupAdded
		);
		this->connect(
			this,
			&Group::sig_aboutToMove,
			db,
			&Database::sig_groupAboutToMove
		);
		this->connect(
			this,
			&Group::sig_moved,
			db,
			&Database::sig_groupMoved
		);
		this->connect(
			this,
			&Group::sig_modified,
			db,
			&Database::sig_modifiedImmediate
		);
	}
	this->db = db;
	for(Group* group_: asConst(
			this->children
		))
	{
		group_->recSetDatabase(
			db
		);
	}
}

void Group::cleanupParent()
{
	if(this->parent)
	{
		sig_aboutToRemove(
			this
		);
		this->parent->children.removeAll(
			this
		);
		sig_modified();
		sig_removed();
	}
}

void Group::recCreateDelObjects()
{
	if(this->db)
	{
		for(const Entry* entry_: asConst(
				this->entries
			))
		{
			this->db->addDeletedObject(
				entry_->getUUID()
			);
		}
		for(Group* group_: asConst(
				this->children
			))
		{
			group_->recCreateDelObjects();
		}
		this->db->addDeletedObject(
			this->uuid
		);
	}
}

bool Group::isResolveSearchingEnabled() const
{
	switch(this->data.searchingEnabled)
	{
		case Inherit:
			if(!this->parent)
			{
				return true;
			}
			return this->parent->isResolveSearchingEnabled();
		case Enable:
			return true;
		case Disable:
			return false;
		default:
			return false;
	}
}
