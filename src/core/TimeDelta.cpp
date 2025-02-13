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
#include "TimeDelta.h"
#include <QDateTime>

QDateTime operator+(
	const QDateTime &dateTime,
	const TimeDelta &delta
)
{
	return dateTime.addDays(
		delta.getDays()
	).addMonths(
		delta.getMonths()
	).addYears(
		delta.getYears()
	);
}

TimeDelta TimeDelta::fromDays(
	const int days
)
{
	return TimeDelta(
		days,
		0,
		0
	);
}

TimeDelta TimeDelta::fromMonths(
	const int months
)
{
	return TimeDelta(
		0,
		months,
		0
	);
}

TimeDelta TimeDelta::fromYears(
	const int years
)
{
	return TimeDelta(
		0,
		0,
		years
	);
}

TimeDelta::TimeDelta()
	: days(
		0
	),
	months(
		0
	),
	years(
		0
	)
{
}

TimeDelta::TimeDelta(
	const int days,
	const int months,
	const int years
)
	: days(
		days
	),
	months(
		months
	),
	years(
		years
	)
{
}

int TimeDelta::getDays() const
{
	return this->days;
}

int TimeDelta::getMonths() const
{
	return this->months;
}

int TimeDelta::getYears() const
{
	return this->years;
}
