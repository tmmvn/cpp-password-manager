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
#ifndef KEEPASSX_RANDOM_H
#define KEEPASSX_RANDOM_H
#include <QByteArray>
#include <QScopedPointer>

class RandomBackend
{
public:
	virtual void randomize(
		void* data,
		int len
	) = 0;

	virtual ~RandomBackend()
	{
	}
};

class Random
{
public:
	void randomize(
		QByteArray &ba
	) const;
	QByteArray getRandomArray(
		int len
	) const;
	/**
	* Generate a random quint32 in the range [0, @p limit)
	*/
	quint32 getRandomUInt(
		quint32 limit
	) const;
	/**
	* Generate a random quint32 in the range [@p min, @p max)
	*/
	quint32 getRandomUIntRange(
		quint32 min,
		quint32 max
	) const;
	static Random* getInstance();
	static void createWithBackend(
		RandomBackend* backend
	);
private:
	explicit Random(
		RandomBackend* backend
	);
	QScopedPointer<RandomBackend> backend;
	static Random* instance;
	Q_DISABLE_COPY(
		Random
	)
};
#endif // KEEPASSX_RANDOM_H
