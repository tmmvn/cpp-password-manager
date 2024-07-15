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
#ifndef KEEPASSX_PASSWORDKEY_H
#define KEEPASSX_PASSWORDKEY_H
#include <QString>
#include "keys/Key.h"

class PasswordKey final:public Key
{
public:
	PasswordKey();
	explicit PasswordKey(
		const QString &password
	);
	virtual QByteArray rawKey() const override;
	void setPassword(
		const QString &password
	);
	virtual PasswordKey* clone() const override;
private:
	QByteArray key;
};
#endif // KEEPASSX_PASSWORDKEY_H
