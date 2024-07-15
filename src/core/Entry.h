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
#ifndef KEEPASSX_ENTRY_H
#define KEEPASSX_ENTRY_H
#include <QColor>
#include <QPixmap>
#include <QPointer>
#include <QUrl>
#include "core/EntryAttachments.h"
#include "core/EntryAttributes.h"
#include "core/TimeInfo.h"
#include "core/UUID.h"
class Database;
class Group;

struct EntryData
{
	int iconNumber;
	UUID customIcon;
	QColor foregroundColor;
	QColor backgroundColor;
	QString overrideUrl;
	QString tags;
	bool autoTypeEnabled;
	int autoTypeObfuscation;
	QString defaultAutoTypeSequence;
	TimeInfo timeInfo;
};

class Entry final:public QObject
{
	Q_OBJECT public:
	Entry();
	virtual ~Entry() override;
	UUID getUUID() const;
	QImage getIcon() const;
	QPixmap getIconPixmap() const;
	QPixmap getIconScaledPixmap() const;
	int getIconNumber() const;
	UUID getIconUUID() const;
	QColor getForegroundColor() const;
	QColor getBackgroundColor() const;
	QString getOverrideURL() const;
	QString getTags() const;
	TimeInfo getTimeInfo() const;
	bool isAutoTypeEnabled() const;
	int getAutoTypeObfuscation() const;
	QString defaultAutoTypeSequence() const;
	QString getTitle() const;
	QString getURL() const;
	QString getUsername() const;
	QString getPassword() const;
	QString getNotes() const;
	bool isExpired() const;
	EntryAttributes* getAttributes();
	const EntryAttributes* getAttributes() const;
	EntryAttachments* getAttachments();
	const EntryAttachments* getAttachments() const;
	static const int DefaultIconNumber;
	void setUUID(
		const UUID &uuid
	);
	void setIcon(
		int iconNumber
	);
	void setIcon(
		const UUID &uuid
	);
	void setForegroundColor(
		const QColor &color
	);
	void setBackgroundColor(
		const QColor &color
	);
	void setOverrideURL(
		const QString &url
	);
	void setTags(
		const QString &tags
	);
	void setTimeInfo(
		const TimeInfo &timeInfo
	);
	void setAutoTypeEnabled(
		bool enable
	);
	void setAutoTypeObfuscation(
		int obfuscation
	);
	void setDefaultAutoTypeSequence(
		const QString &sequence
	);
	void setTitle(
		const QString &title
	) const;
	void setURL(
		const QString &url
	) const;
	void setUsername(
		const QString &username
	) const;
	void setPassword(
		const QString &password
	) const;
	void setNotes(
		const QString &notes
	) const;
	void setExpires(
		const bool &value
	);
	void setExpiryTime(
		const QDateTime &dateTime
	);
	QList<Entry*> getHistoryItems();
	const QList<Entry*> &getHistoryItems() const;
	void addHistoryItem(
		Entry* entry
	);
	void removeHistoryItems(
		const QList<Entry*> &historyEntries
	);
	void truncateHistory();

	enum CloneFlag: u_int8_t
	{
		CloneNoFlags = 0,
		CloneNewUuid = 1,
		// generate a random uuid for the clone
		CloneResetTimeInfo = 2,
		// set all TimeInfo attributes to the current time
		CloneIncludeHistory = 4 // clone the history items
	};

	Q_DECLARE_FLAGS(
		CloneFlags,
		CloneFlag
	)
	/**
	* Creates a duplicate of this entry except that the returned entry isn't
	* part of any group.
	* Note that you need to copy the custom icons manually when inserting the
	* new entry into another database.
	*/
	Entry* clone(
		CloneFlags flags
	) const;
	void copyDataFrom(
		const Entry* other
	);
	QString resolvePlaceholders(
		const QString &str
	) const;
	/**
	* Call before and after set*() methods to create a history item
	* if the entry has been changed.
	*/
	void beginUpdate();
	bool endUpdate();
	Group* getGroup();
	const Group* getGroup() const;
	void setGroup(
		Group* group
	);
	void setUpdateTimeinfo(
		bool value
	);
Q_SIGNALS:
	/**
	* Emitted when a default attribute has been changed.
	*/
	void sig_dataChanged(
		Entry* entry
	);
	void sig_modified();
private Q_SLOTS:
	void do_emitDataChanged();
	void do_updateTimeinfo();
	void do_updateModifiedSinceBegin();
private:
	const Database* getDatabase() const;
	template<class T> bool set(
		T &property,
		const T &value
	);
	UUID uuid;
	EntryData data;
	EntryAttributes* const attributes;
	EntryAttachments* const attachments;
	QList<Entry*> history;
	Entry* tmpHistoryItem;
	bool modifiedSinceBegin;
	QPointer<Group> group;
	bool updateTimeinfo;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(
	Entry::CloneFlags
)
#endif // KEEPASSX_ENTRY_H
