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
#include "TimeInfo.h"
#include "core/Tools.h"

TimeInfo::TimeInfo()
	: expires(
		false
	),
	usageCount(
		0
	)
{
	const QDateTime now_ = QDateTime::currentDateTimeUtc();
	this->lastModificationTime = now_;
	this->creationTime = now_;
	this->lastAccessTime = now_;
	this->expiryTime = now_;
	this->locationChanged = now_;
}

QDateTime TimeInfo::getLastModificationTime() const
{
	return this->lastModificationTime;
}

QDateTime TimeInfo::getCreationTime() const
{
	return this->creationTime;
}

QDateTime TimeInfo::getLastAccessTime() const
{
	return this->lastAccessTime;
}

QDateTime TimeInfo::getExpiryTime() const
{
	return this->expiryTime;
}

bool TimeInfo::getExpires() const
{
	return this->expires;
}

int TimeInfo::getUsageCount() const
{
	return this->usageCount;
}

QDateTime TimeInfo::getLocationChanged() const
{
	return this->locationChanged;
}

void TimeInfo::setLastModificationTime(
	const QDateTime &dateTime
)
{
	if(dateTime.timeSpec() != Qt::UTC)
	{
		return;
	}
	this->lastModificationTime = dateTime;
}

void TimeInfo::setCreationTime(
	const QDateTime &dateTime
)
{
	if(dateTime.timeSpec() != Qt::UTC)
	{
		return;
	}
	this->creationTime = dateTime;
}

void TimeInfo::setLastAccessTime(
	const QDateTime &dateTime
)
{
	if(dateTime.timeSpec() != Qt::UTC)
	{
		return;
	}
	this->lastAccessTime = dateTime;
}

void TimeInfo::setExpiryTime(
	const QDateTime &dateTime
)
{
	if(dateTime.timeSpec() != Qt::UTC)
	{
		return;
	}
	this->expiryTime = dateTime;
}

void TimeInfo::setExpires(
	const bool expires
)
{
	this->expires = expires;
}

void TimeInfo::setUsageCount(
	const int count
)
{
	this->usageCount = count;
}

void TimeInfo::setLocationChanged(
	const QDateTime &dateTime
)
{
	if(dateTime.timeSpec() != Qt::UTC)
	{
		return;
	}
	this->locationChanged = dateTime;
}
