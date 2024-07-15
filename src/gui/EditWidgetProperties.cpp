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
#include "EditWidgetProperties.h"
#include "ui_EditWidgetProperties.h"

EditWidgetProperties::EditWidgetProperties(
	QWidget* parent
)
	: QWidget(
		parent
	),
	ui(
		new Ui::EditWidgetProperties()
	)
{
	this->ui->setupUi(
		this
	);
}

EditWidgetProperties::~EditWidgetProperties()
{
}

void EditWidgetProperties::setFields(
	const TimeInfo &timeInfo,
	const UUID &uuid
) const
{
	const QString timeFormat_(
		"d MMM yyyy HH:mm:ss"
	);
	this->ui->modifiedEdit->setText(
		timeInfo.getLastModificationTime().toLocalTime().toString(
			timeFormat_
		)
	);
	this->ui->createdEdit->setText(
		timeInfo.getCreationTime().toLocalTime().toString(
			timeFormat_
		)
	);
	this->ui->accessedEdit->setText(
		timeInfo.getLastAccessTime().toLocalTime().toString(
			timeFormat_
		)
	);
	this->ui->uuidEdit->setText(
		uuid.toHex()
	);
}
