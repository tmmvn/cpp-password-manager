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
#include "UUID.h"
#include <QHash>
#include "crypto/Random.h"
const int UUID::Length = 16;

UUID::UUID()
	: data(
		Length,
		0
	)
{
}

UUID::UUID(
	const QByteArray &data
)
{
	if(data.size() != this->Length)
	{
		return;
	}
	this->data = data;
}

UUID UUID::random()
{
	return UUID(
		Random::getInstance()->getRandomArray(
			Length
		)
	);
}

QString UUID::toBase64() const
{
	return QString::fromLatin1(
		this->data.toBase64()
	);
}

QString UUID::toHex() const
{
	return QString::fromLatin1(
		this->data.toHex()
	);
}

QByteArray UUID::toByteArray() const
{
	return this->data;
}

bool UUID::isNull() const
{
	for(auto i_ = 0; i_ < this->data.size(); ++i_)
	{
		if(this->data[i_] != 0)
		{
			return false;
		}
	}
	return true;
}

// UUID &UUID::operator=(
// 	const UUID &other
// )
// {
// 	this->data = other.data;
// 	return *this;
// }
bool UUID::operator==(
	const UUID &other
) const
{
	return this->data == other.data;
}

bool UUID::operator!=(
	const UUID &other
) const
{
	return !operator==(
		other
	);
}

UUID UUID::fromBase64(
	const QString &str
)
{
	const QByteArray data_ = QByteArray::fromBase64(
		str.toLatin1()
	);
	return UUID(
		data_
	);
}

uint qHash(
	const UUID &key
)
{
	return qHash(
		key.toByteArray()
	);
}

QDataStream &operator<<(
	QDataStream &stream,
	const UUID &uuid
)
{
	return stream << uuid.toByteArray();
}

QDataStream &operator>>(
	QDataStream &stream,
	UUID &uuid
)
{
	QByteArray data_;
	stream >> data_;
	if(data_.size() == UUID::Length)
	{
		uuid = UUID(
			data_
		);
	}
	return stream;
}
