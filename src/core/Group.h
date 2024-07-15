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
#ifndef KEEPASSX_GROUP_H
#define KEEPASSX_GROUP_H
#include <QImage>
#include <QPixmapCache>
#include "core/Database.h"
#include "core/Entry.h"
#include "core/TimeInfo.h"
#include "core/UUID.h"

class Group final:public QObject
{
	Q_OBJECT public:
	enum TriState: u_int8_t
	{
		Inherit,
		Enable,
		Disable
	};

	struct GroupData
	{
		QString name;
		QString notes;
		int iconNumber;
		UUID customIcon;
		TimeInfo timeInfo;
		bool isExpanded;
		QString defaultAutoTypeSequence;
		Group::TriState autoTypeEnabled;
		Group::TriState searchingEnabled;
	};

	Group();
	virtual ~Group() override;
	static Group* createRecycleBin();
	UUID getUUID() const;
	QString getName() const;
	QString getNotes() const;
	QImage getIcon() const;
	QPixmap getIconPixmap() const;
	QPixmap getIconScaledPixmap() const;
	int getIconNumber() const;
	UUID getIconUUID() const;
	TimeInfo getTimeInfo() const;
	bool isExpanded() const;
	QString getDefaultAutoTypeSequence() const;
	TriState isAutoTypeEnabled() const;
	TriState isSearchingEnabled() const;
	bool isResolveSearchingEnabled() const;
	bool isResolveAutoTypeEnabled() const;
	Entry* getLastTopVisibleEntry() const;
	bool isExpired() const;
	static const int DefaultIconNumber;
	static const int RecycleBinIconNumber;
	void setUuid(
		const UUID &uuid
	);
	void setName(
		const QString &name
	);
	void setNotes(
		const QString &notes
	);
	void setIcon(
		int iconNumber
	);
	void setIcon(
		const UUID &uuid
	);
	void setTimeInfo(
		const TimeInfo &timeInfo
	);
	void setExpanded(
		bool expanded
	);
	void setDefaultAutoTypeSequence(
		const QString &sequence
	);
	void setAutoTypeEnabled(
		TriState enable
	);
	void setSearchingEnabled(
		TriState enable
	);
	void setLastTopVisibleEntry(
		Entry* entry
	);
	void setExpires(
		bool value
	);
	void setExpiryTime(
		const QDateTime &dateTime
	);
	void setUpdateTimeinfo(
		bool value
	);
	Group* getParentGroup();
	const Group* getParentGroup() const;
	void setParent(
		Group* parent,
		int index = -1
	);
	Database* getDatabase();
	const Database* getDatabase() const;
	QList<Group*> getChildren();
	const QList<Group*> &getChildren() const;
	QList<Entry*> getEntries();
	const QList<Entry*> &getEntries() const;
	QList<Entry*> getEntriesRecursive(
		bool includeHistoryItems = false
	) const;
	QList<const Group*> getGroupsRecursive(
		bool includeSelf
	) const;
	QList<Group*> getGroupsRecursive(
		bool includeSelf
	);
	QSet<UUID> getCustomIconsRecursive() const;
	/**
	* Creates a duplicate of this group including all child entries and groups.
	* The exceptions are that the returned group doesn't have a parent group
	* and all TimeInfo attributes are set to the current time.
	* Note that you need to copy the custom icons manually when inserting the
	* new group into another database.
	*/
	Group* clone(
		Entry::CloneFlags entryFlags = Entry::CloneNewUuid |
			Entry::CloneResetTimeInfo
	) const;
	void copyDataFrom(
		const Group* other
	);
Q_SIGNALS:
	void sig_dataChanged(
		Group* group
	);
	void sig_aboutToAdd(
		Group* group,
		int index
	);
	void sig_added();
	void sig_aboutToRemove(
		Group* group
	);
	void sig_removed();
	/**
	* Group moved within the database.
	*/
	void sig_aboutToMove(
		Group* group,
		Group* toGroup,
		int index
	);
	void sig_moved();
	void sig_entryAboutToAdd(
		Entry* entry
	);
	void sig_entryAdded(
		Entry* entry
	);
	void sig_entryAboutToRemove(
		Entry* entry
	);
	void sig_entryRemoved(
		Entry* entry
	);
	void sig_entryDataChanged(
		Entry* entry
	);
	void sig_modified();
private:
	template<class P, class V> bool set(
		P &property,
		const V &value
	);
	void addEntry(
		Entry* entry
	);
	void removeEntry(
		Entry* entry
	);
	void setParent(
		Database* db
	);
	void recSetDatabase(
		Database* db
	);
	void cleanupParent();
	void recCreateDelObjects();
	void getUpdateTimeinfo();
	QPointer<Database> db;
	UUID uuid;
	GroupData data;
	QPointer<Entry> lastTopVisibleEntry;
	QList<Group*> children;
	QList<Entry*> entries;
	QPointer<Group> parent;
	bool updateTimeinfo;
	friend void Database::setRootGroup(
		Group* group
	);
	friend Entry::~Entry();
	friend void Entry::setGroup(
		Group* group
	);
};
#endif // KEEPASSX_GROUP_H
