/*
 *  Copyright (C) 2012 Felix Geyer <debfx@fobos.de>
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
#ifndef KEEPASSX_ENTRYATTACHMENTS_H
#define KEEPASSX_ENTRYATTACHMENTS_H
#include <QMap>
#include <QObject>

class EntryAttachments final:public QObject
{
	Q_OBJECT public:
	explicit EntryAttachments(
		QObject* parent = nullptr
	);
	QList<QString> getKeys() const;
	bool hasKey(
		const QString &key
	) const;
	QList<QByteArray> getValues() const;
	QByteArray getValue(
		const QString &key
	) const;
	void set(
		const QString &key,
		const QByteArray &value
	);
	void remove(
		const QString &key
	);
	void clear();
	void copyDataFrom(
		const EntryAttachments* other
	);
	bool operator==(
		const EntryAttachments &other
	) const;
	bool operator!=(
		const EntryAttachments &other
	) const;
Q_SIGNALS:
	void sig_modified();
	void sig_keyModified(
		const QString &key
	);
	void sig_aboutToBeAdded(
		const QString &key
	);
	void sig_added(
		const QString &key
	);
	void sig_aboutToBeRemoved(
		const QString &key
	);
	void sig_removed(
		const QString &key
	);
	void sig_aboutToBeReset();
	void sig_reset();
private:
	QMap<QString, QByteArray> attachments;
};
#endif // KEEPASSX_ENTRYATTACHMENTS_H
