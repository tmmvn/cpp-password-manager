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
#ifndef KEEPASSX_UUID_H
#define KEEPASSX_UUID_H
#include <QByteArray>
#include <QString>

class UUID
{
public:
	UUID();
	explicit UUID(
		const QByteArray &data
	);
	static UUID random();
	QString toBase64() const;
	QString toHex() const;
	QByteArray toByteArray() const;
	bool isNull() const;
	// UUID &operator=(
	// 	const UUID &other
	// );
	bool operator==(
		const UUID &other
	) const;
	bool operator!=(
		const UUID &other
	) const;
	static const int Length;
	static UUID fromBase64(
		const QString &str
	);
private:
	QByteArray data;
};

Q_DECLARE_TYPEINFO(
	UUID,
	Q_MOVABLE_TYPE
);

uint qHash(
	const UUID &key
);
QDataStream &operator<<(
	QDataStream &stream,
	const UUID &uuid
);
QDataStream &operator>>(
	QDataStream &stream,
	UUID &uuid
);
#endif // KEEPASSX_UUID_H
