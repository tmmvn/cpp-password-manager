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
#ifndef KEEPASSX_PASSWORDGENERATOR_H
#define KEEPASSX_PASSWORDGENERATOR_H
#include <QString>
#include <QVector>
typedef QVector<QChar> PasswordGroup;

class PasswordGenerator
{
public:
	enum CharClass: u_int8_t
	{
		LowerLetters = 0x1,
		UpperLetters = 0x2,
		Numbers = 0x4,
		SpecialCharacters = 0x8
	};

	Q_DECLARE_FLAGS(
		CharClasses,
		CharClass
	)

	enum GeneratorFlag: u_int8_t
	{
		ExcludeLookAlike = 0x1,
		CharFromEveryGroup = 0x2
	};

	Q_DECLARE_FLAGS(
		GeneratorFlags,
		GeneratorFlag
	)
public:
	PasswordGenerator();
	void setLength(
		int length
	);
	void setCharClasses(
		const CharClasses &classes
	);
	void setFlags(
		const GeneratorFlags &flags
	);
	bool isValid() const;
	QString generatePassword() const;
private:
	QVector<PasswordGroup> getPasswordGroups() const;
	int numCharClasses() const;
	int length;
	CharClasses classes;
	GeneratorFlags flags;
	Q_DISABLE_COPY(
		PasswordGenerator
	)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(
	PasswordGenerator::CharClasses
)

Q_DECLARE_OPERATORS_FOR_FLAGS(
	PasswordGenerator::GeneratorFlags
)
#endif // KEEPASSX_PASSWORDGENERATOR_H
