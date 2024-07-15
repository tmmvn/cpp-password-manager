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
#include "EntryAttributes.h"
const QString EntryAttributes::TitleKey = "Title";
const QString EntryAttributes::UserNameKey = "UserName";
const QString EntryAttributes::PasswordKey = "Password";
const QString EntryAttributes::URLKey = "URL";
const QString EntryAttributes::NotesKey = "Notes";
const QStringList EntryAttributes::DefaultAttributes(
	QStringList() << TitleKey << UserNameKey << PasswordKey << URLKey <<
	NotesKey
);

EntryAttributes::EntryAttributes(
	QObject* parent
)
	: QObject(
		parent
	)
{
	this->clear();
}

QList<QString> EntryAttributes::getKeys() const
{
	return this->attributes.keys();
}

bool EntryAttributes::hasKey(
	const QString &key
) const
{
	return this->attributes.keys().contains(
		key
	);
}

QList<QString> EntryAttributes::getCustomKeys() const
{
	QList<QString> customKeys_;
	const QList<QString> keyList_ = this->getKeys();
	for(const QString &key_: keyList_)
	{
		if(!this->isDefaultAttribute(
			key_
		))
		{
			customKeys_.append(
				key_
			);
		}
	}
	return customKeys_;
}

QString EntryAttributes::getValue(
	const QString &key
) const
{
	return this->attributes.value(
		key
	);
}

bool EntryAttributes::isProtected(
	const QString &key
) const
{
	return this->protectedAttributes.contains(
		key
	);
}

void EntryAttributes::set(
	const QString &key,
	const QString &value,
	const bool protect
)
{
	auto emitModified_ = false;
	const bool addAttribute_ = !this->attributes.contains(
		key
	);
	const bool changeValue_ = !addAttribute_ && (this->attributes.value(
		key
	) != value);
	const bool defaultAttribute_ = this->isDefaultAttribute(
		key
	);
	if(addAttribute_ && !defaultAttribute_)
	{
		sig_aboutToBeAdded(
			key
		);
	}
	if(addAttribute_ || changeValue_)
	{
		this->attributes.insert(
			key,
			value
		);
		emitModified_ = true;
	}
	if(protect)
	{
		if(!this->protectedAttributes.contains(
			key
		))
		{
			emitModified_ = true;
		}
		this->protectedAttributes.insert(
			key
		);
	}
	else if(this->protectedAttributes.remove(
		key
	))
	{
		emitModified_ = true;
	}
	if(emitModified_)
	{
		sig_modified();
	}
	if(defaultAttribute_ && changeValue_)
	{
		sig_defaultKeyModified();
	}
	else if(addAttribute_)
	{
		sig_added(
			key
		);
	}
	else if(emitModified_)
	{
		sig_customKeyModified(
			key
		);
	}
}

void EntryAttributes::remove(
	const QString &key
)
{
	if(this->isDefaultAttribute(
		key
	))
	{
		return;
	};
	if(!this->attributes.contains(
		key
	))
	{
		return;
	}
	sig_aboutToBeRemoved(
		key
	);
	this->attributes.remove(
		key
	);
	this->protectedAttributes.remove(
		key
	);
	sig_removed(
		key
	);
	sig_modified();
}

void EntryAttributes::rename(
	const QString &oldKey,
	const QString &newKey
)
{
	if(this->isDefaultAttribute(
		oldKey
	))
	{
		return;
	};
	if(this->isDefaultAttribute(
		newKey
	))
	{
		return;
	};
	if(this->attributes.contains(
		oldKey
	))
	{
		return;
	};
	if(!this->attributes.contains(
		oldKey
	))
	{
		return;
	}
	if(!this->attributes.contains(
		newKey
	))
	{
		return;
	};
	if(this->attributes.contains(
		newKey
	))
	{
		return;
	}
	const QString data_ = this->getValue(
		oldKey
	);
	const bool protect_ = this->isProtected(
		oldKey
	);
	sig_aboutToRename(
		oldKey,
		newKey
	);
	this->attributes.remove(
		oldKey
	);
	this->attributes.insert(
		newKey,
		data_
	);
	if(protect_)
	{
		this->protectedAttributes.remove(
			oldKey
		);
		this->protectedAttributes.insert(
			newKey
		);
	}
	sig_modified();
	sig_renamed(
		oldKey,
		newKey
	);
}

void EntryAttributes::copyCustomKeysFrom(
	const EntryAttributes* other
)
{
	if(!this->areCustomKeysDifferent(
		other
	))
	{
		return;
	}
	sig_aboutToBeReset();
	// remove all non-default keys
	const QList<QString> keyList_ = this->getKeys();
	for(const QString &key_: keyList_)
	{
		if(!this->isDefaultAttribute(
			key_
		))
		{
			this->attributes.remove(
				key_
			);
			this->protectedAttributes.remove(
				key_
			);
		}
	}
	const QList<QString> otherKeyList_ = other->getKeys();
	for(const QString &key_: otherKeyList_)
	{
		if(!this->isDefaultAttribute(
			key_
		))
		{
			this->attributes.insert(
				key_,
				other->getValue(
					key_
				)
			);
			if(other->isProtected(
				key_
			))
			{
				this->protectedAttributes.insert(
					key_
				);
			}
		}
	}
	sig_reset();
	sig_modified();
}

bool EntryAttributes::areCustomKeysDifferent(
	const EntryAttributes* other
) const
{
	if(other == nullptr)
	{
		return true;
	}
	if(this->getKeys() != other->getKeys())
	{
		return true;
	}
	const QList<QString> keyList_ = this->getKeys();
	for(const QString &key_: keyList_)
	{
		if(this->isDefaultAttribute(
			key_
		))
		{
			continue;
		}
		if(this->isProtected(
			key_
		) != other->isProtected(
			key_
		) || this->getValue(
			key_
		) != other->getValue(
			key_
		))
		{
			return true;
		}
	}
	return false;
}

void EntryAttributes::copyDataFrom(
	const EntryAttributes* other
)
{
	if(*this != *other)
	{
		sig_aboutToBeReset();
		this->attributes = other->attributes;
		this->protectedAttributes = other->protectedAttributes;
		sig_reset();
		sig_modified();
	}
}

bool EntryAttributes::operator==(
	const EntryAttributes &other
) const
{
	return (this->attributes == other.attributes && this->protectedAttributes ==
		other.protectedAttributes);
}

bool EntryAttributes::operator!=(
	const EntryAttributes &other
) const
{
	return (this->attributes != other.attributes || this->protectedAttributes !=
		other.protectedAttributes);
}

void EntryAttributes::clear()
{
	sig_aboutToBeReset();
	this->attributes.clear();
	this->protectedAttributes.clear();
	for(const QString &key_: this->DefaultAttributes)
	{
		this->attributes.insert(
			key_,
			""
		);
	}
	sig_reset();
	sig_modified();
}

int EntryAttributes::getAttributesSize() const
{
	auto size_ = 0;
	QMapIterator i_(
		this->attributes
	);
	while(i_.hasNext())
	{
		i_.next();
		size_ += static_cast<int>(i_.value().toUtf8().size());
	}
	return size_;
}

bool EntryAttributes::isDefaultAttribute(
	const QString &key
)
{
	return DefaultAttributes.contains(
		key
	);
}
