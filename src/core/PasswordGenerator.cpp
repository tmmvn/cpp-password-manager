/*
 *  Copyright (C) 2013 Felix Geyer <debfx@fobos.de>
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
#include "PasswordGenerator.h"
#include "crypto/Random.h"

PasswordGenerator::PasswordGenerator()
	: length(
		0
	),
	classes(
		0
	),
	flags(
		0
	)
{
}

void PasswordGenerator::setLength(
	const int length
)
{
	this->length = length;
}

void PasswordGenerator::setCharClasses(
	const CharClasses &classes
)
{
	this->classes = classes;
}

void PasswordGenerator::setFlags(
	const GeneratorFlags &flags
)
{
	this->flags = flags;
}

QString PasswordGenerator::generatePassword() const
{
	if(!this->isValid())
	{
		return QString();
	}
	const QVector<PasswordGroup> groups_ = this->getPasswordGroups();
	QVector<QChar> passwordChars_;
	for(const PasswordGroup &group_: groups_)
	{
		for(QChar ch_: group_)
		{
			passwordChars_.append(
				ch_
			);
		}
	}
	QString password_;
	if(this->flags & this->CharFromEveryGroup)
	{
		for(auto i_ = 0; i_ < groups_.size(); i_++)
		{
			const quint32 pos_ = Random::getInstance()->getRandomUInt(
				groups_[i_].size()
			);
			password_.append(
				groups_[i_][pos_]
			);
		}
		for(qsizetype i_ = groups_.size(); i_ < length; i_++)
		{
			const quint32 pos_ = Random::getInstance()->getRandomUInt(
				passwordChars_.size()
			);
			password_.append(
				passwordChars_[pos_]
			);
		}
		// shuffle chars
		for(qsizetype i_ = (password_.size() - 1); i_ >= 1; i_--)
		{
			const quint32 j_ = Random::getInstance()->getRandomUInt(
				i_ + 1
			);
			const QChar tmp_ = password_[i_];
			password_[i_] = password_[j_];
			password_[j_] = tmp_;
		}
	}
	else
	{
		for(auto i_ = 0; i_ < length; i_++)
		{
			const quint32 pos = Random::getInstance()->getRandomUInt(
				passwordChars_.size()
			);
			password_.append(
				passwordChars_[pos]
			);
		}
	}
	return password_;
}

bool PasswordGenerator::isValid() const
{
	if(this->classes == 0)
	{
		return false;
	}
	if(this->length == 0)
	{
		return false;
	}
	if((this->flags & this->CharFromEveryGroup) && (this->length < this->
		numCharClasses()))
	{
		return false;
	}
	return true;
}

QVector<PasswordGroup> PasswordGenerator::getPasswordGroups() const
{
	QVector<PasswordGroup> passwordGroups_;
	if(this->classes & this->LowerLetters)
	{
		PasswordGroup group_;
		for(auto i_ = 97; i_ < (97 + 26); i_++)
		{
			if(this->flags & this->ExcludeLookAlike && i_ == 108)
			{
				// "l"
				continue;
			}
			group_.append(
				static_cast<QChar>(i_)
			);
		}
		passwordGroups_.append(
			group_
		);
	}
	if(this->classes & this->UpperLetters)
	{
		PasswordGroup group_;
		for(auto i_ = 65; i_ < (65 + 26); i_++)
		{
			if(this->flags & this->ExcludeLookAlike && (i_ == 73 || i_ == 79))
			{
				// "I" and "O"
				continue;
			}
			group_.append(
				static_cast<QChar>(i_)
			);
		}
		passwordGroups_.append(
			group_
		);
	}
	if(this->classes & this->Numbers)
	{
		PasswordGroup group_;
		for(auto i_ = 48; i_ < (48 + 10); i_++)
		{
			if(this->flags & this->ExcludeLookAlike && (i_ == 48 || i_ == 49))
			{
				// "0" and "1"
				continue;
			}
			group_.append(
				static_cast<QChar>(i_)
			);
		}
		passwordGroups_.append(
			group_
		);
	}
	if(this->classes & this->SpecialCharacters)
	{
		PasswordGroup group_;
		for(auto i_ = 33; i_ <= 47; i_++)
		{
			group_.append(
				static_cast<QChar>(i_)
			);
		}
		for(auto i_ = 58; i_ <= 64; i_++)
		{
			group_.append(
				static_cast<QChar>(i_)
			);
		}
		for(auto i_ = 91; i_ <= 96; i_++)
		{
			group_.append(
				static_cast<QChar>(i_)
			);
		}
		for(auto i_ = 123; i_ <= 126; i_++)
		{
			if(this->flags & this->ExcludeLookAlike && i_ == 124)
			{
				// "|"
				continue;
			}
			group_.append(
				static_cast<QChar>(i_)
			);
		}
		passwordGroups_.append(
			group_
		);
	}
	return passwordGroups_;
}

int PasswordGenerator::numCharClasses() const
{
	auto numClasses_ = 0;
	if(this->classes & this->LowerLetters)
	{
		numClasses_++;
	}
	if(this->classes & this->UpperLetters)
	{
		numClasses_++;
	}
	if(this->classes & this->Numbers)
	{
		numClasses_++;
	}
	if(this->classes & this->SpecialCharacters)
	{
		numClasses_++;
	}
	return numClasses_;
}
