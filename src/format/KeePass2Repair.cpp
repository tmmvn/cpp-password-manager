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
#include "KeePass2Repair.h"
#include <QBuffer>
#include <QRegularExpression>
#include "format/KeePass2RandomStream.h"
#include "format/KeePass2Reader.h"
#include "format/KeePass2XmlReader.h"

KeePass2Repair::KeePass2Repair()
	: db(
		nullptr
	)
{
}

KeePass2Repair::RepairResult KeePass2Repair::repairDatabase(
	QIODevice* device,
	const CompositeKey &key
)
{
	this->db = nullptr;
	this->errorStr.clear();
	KeePass2Reader reader_;
	reader_.setSaveXml(
		true
	);
	Database* db_ = reader_.readDatabase(
		device,
		key,
		true
	);
	if(!reader_.hasError())
	{
		delete db_;
		return NothingTodo;
	}
	QByteArray xmlData_ = reader_.getXMLData();
	if(!db_ || xmlData_.isEmpty())
	{
		delete db_;
		this->errorStr = reader_.getErrorString();
		return UnableToOpen;
	}
	auto repairAction_ = false;
	QString xmlStart_ = QString::fromLatin1(
		xmlData_.constData(),
		qMin(
			100,
			xmlData_.size()
		)
	);
	QRegularExpression encodingRegExp_(
		"encoding=\"([^\"]+)\"",
		QRegularExpression::CaseInsensitiveOption
	);
	if(QRegularExpressionMatch match_ = encodingRegExp_.match(
			xmlStart_
		);
		match_.hasMatch())
	{
		if(match_.captured(
			1
		).compare(
			"utf-8",
			Qt::CaseInsensitive
		) != 0 && match_.captured(
			1
		).compare(
			"utf8",
			Qt::CaseInsensitive
		) != 0)
		{
			// database is not utf-8 encoded, we don't support repairing that
			delete db_;
			return RepairFailed;
		}
	}
	// try to fix broken databases because of bug #392
	for(qsizetype i_ = xmlData_.size() - 1; i_ >= 0; i_--)
	{
		if(auto ch_ = static_cast<quint8>(xmlData_.at(
				i_
			));
			ch_ < 0x20 && ch_ != 0x09 && ch_ != 0x0A && ch_ != 0x0D)
		{
			xmlData_.remove(
				i_,
				1
			);
			repairAction_ = true;
		}
	}
	if(!repairAction_)
	{
		// we were unable to find the problem
		delete db_;
		return RepairFailed;
	}
	KeePass2RandomStream randomStream_;
	randomStream_.init(
		reader_.getStreamKey()
	);
	KeePass2XmlReader xmlReader_;
	QBuffer buffer_(
		&xmlData_
	);
	buffer_.open(
		QIODevice::ReadOnly
	);
	xmlReader_.readDatabase(
		&buffer_,
		db_,
		&randomStream_
	);
	if(xmlReader_.hasError())
	{
		delete db_;
		return RepairFailed;
	}
	this->db = db_;
	return RepairSuccess;
}

Database* KeePass2Repair::getDatabase() const
{
	return this->db;
}

QString KeePass2Repair::getErrorString() const
{
	return this->errorStr;
}
