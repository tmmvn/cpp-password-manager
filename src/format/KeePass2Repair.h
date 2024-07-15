/*
 *  Copyright (C) 2016 Felix Geyer <debfx@fobos.de>
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
#ifndef KEEPASSX_KEEPASS2REPAIR_H
#define KEEPASSX_KEEPASS2REPAIR_H
#include <QCoreApplication>
#include <QIODevice>
#include "core/Database.h"
#include "keys/CompositeKey.h"

class KeePass2Repair
{
	Q_DECLARE_TR_FUNCTIONS(
		KeePass2Repair
	)
public:
	enum RepairResult: u_int8_t
	{
		NothingTodo,
		UnableToOpen,
		RepairSuccess,
		RepairFailed
	};

	KeePass2Repair();
	RepairResult repairDatabase(
		QIODevice* device,
		const CompositeKey &key
	);
	Database* getDatabase() const;
	QString getErrorString() const;
private:
	Database* db;
	QString errorStr;
};
#endif // KEEPASSX_KEEPASS2REPAIR_H
