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
#include "EntryAttachments.h"

EntryAttachments::EntryAttachments(
	QObject* parent
)
	: QObject(
		parent
	)
{
}

QList<QString> EntryAttachments::getKeys() const
{
	return this->attachments.keys();
}

bool EntryAttachments::hasKey(
	const QString &key
) const
{
	return this->attachments.keys().contains(
		key
	);
}

QList<QByteArray> EntryAttachments::getValues() const
{
	return this->attachments.values();
}

QByteArray EntryAttachments::getValue(
	const QString &key
) const
{
	return this->attachments.value(
		key
	);
}

void EntryAttachments::set(
	const QString &key,
	const QByteArray &value
)
{
	auto emitModified_ = false;
	const bool addAttachment_ = !this->attachments.contains(
		key
	);
	if(addAttachment_)
	{
		 sig_aboutToBeAdded(
			key
		);
	}
	if(addAttachment_ || this->attachments.value(
		key
	) != value)
	{
		this->attachments.insert(
			key,
			value
		);
		emitModified_ = true;
	}
	if(addAttachment_)
	{
		 sig_added(
			key
		);
	}
	else
	{
		 sig_keyModified(
			key
		);
	}
	if(emitModified_)
	{
		 sig_modified();
	}
}

void EntryAttachments::remove(
	const QString &key
)
{
	if(this->attachments.contains(
		key
	))
	{
		return;
	};
	if(!this->attachments.contains(
		key
	))
	{
		return;
	}
	 sig_aboutToBeRemoved(
		key
	);
	this->attachments.remove(
		key
	);
	 sig_removed(
		key
	);
	 sig_modified();
}

void EntryAttachments::clear()
{
	if(this->attachments.isEmpty())
	{
		return;
	}
	 sig_aboutToBeReset();
	this->attachments.clear();
	 sig_reset();
	 sig_modified();
}

void EntryAttachments::copyDataFrom(
	const EntryAttachments* other
)
{
	if(*this != *other)
	{
		 sig_aboutToBeReset();
		this->attachments = other->attachments;
		 sig_reset();
		 sig_modified();
	}
}

bool EntryAttachments::operator==(
	const EntryAttachments &other
) const
{
	return this->attachments == other.attachments;
}

bool EntryAttachments::operator!=(
	const EntryAttachments &other
) const
{
	return this->attachments != other.attachments;
}
