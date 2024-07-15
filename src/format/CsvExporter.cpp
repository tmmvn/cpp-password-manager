/*
 *  Copyright (C) 2015 Florian Geyer <blueice@fobos.de>
 *  Copyright (C) 2015 Felix Geyer <debfx@fobos.de>
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
#include "CsvExporter.h"
#include <QFile>
#include "core/Database.h"
#include "core/Group.h"

bool CsvExporter::exportDatabase(
	const QString &filename,
	const Database* db
)
{
	QFile file_(
		filename
	);
	if(!file_.open(
		QIODevice::WriteOnly | QIODevice::Truncate
	))
	{
		this->error = file_.errorString();
		return false;
	}
	return this->exportDatabase(
		&file_,
		db
	);
}

bool CsvExporter::exportDatabase(
	QIODevice* device,
	const Database* db
)
{
	QString header_;
	this->addColumn(
		header_,
		"Group"
	);
	this->addColumn(
		header_,
		"Title"
	);
	this->addColumn(
		header_,
		"Username"
	);
	this->addColumn(
		header_,
		"Password"
	);
	this->addColumn(
		header_,
		"URL"
	);
	this->addColumn(
		header_,
		"Notes"
	);
	header_.append(
		"\n"
	);
	if(device->write(
		header_.toUtf8()
	) == -1)
	{
		this->error = device->errorString();
		return false;
	}
	return this->writeGroup(
		device,
		db->getRootGroup()
	);
}

QString CsvExporter::getErrorString() const
{
	return this->error;
}

bool CsvExporter::writeGroup(
	QIODevice* device,
	const Group* group,
	QString groupPath
)
{
	if(!groupPath.isEmpty())
	{
		groupPath.append(
			"/"
		);
	}
	groupPath.append(
		group->getName()
	);
	const QList<Entry*> &entryList_ = group->getEntries();
	for(const Entry* entry_: entryList_)
	{
		QString line_;
		this->addColumn(
			line_,
			groupPath
		);
		this->addColumn(
			line_,
			entry_->getTitle()
		);
		this->addColumn(
			line_,
			entry_->getUsername()
		);
		this->addColumn(
			line_,
			entry_->getPassword()
		);
		this->addColumn(
			line_,
			entry_->getURL()
		);
		this->addColumn(
			line_,
			entry_->getNotes()
		);
		line_.append(
			"\n"
		);
		if(device->write(
			line_.toUtf8()
		) == -1)
		{
			this->error = device->errorString();
			return false;
		}
	}
	const QList<Group*> &children_ = group->getChildren();
	for(const Group* child_: children_)
	{
		if(!this->writeGroup(
			device,
			child_,
			groupPath
		))
		{
			return false;
		}
	}
	return true;
}

void CsvExporter::addColumn(
	QString &str,
	const QString &column
)
{
	if(!str.isEmpty())
	{
		str.append(
			","
		);
	}
	str.append(
		"\""
	);
	str.append(
		QString(
			column
		).replace(
			"\"",
			"\"\""
		)
	);
	str.append(
		"\""
	);
}
