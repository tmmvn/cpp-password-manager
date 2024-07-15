/*
 *  Copyright (C) 2011 Felix Geyer <debfx@fobos.de>
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
#include "EditGroupWidget.h"
#include "ui_EditGroupWidgetMain.h"
#include "core/Metadata.h"
#include "gui/EditWidgetIcons.h"
#include "gui/EditWidgetProperties.h"

EditGroupWidget::EditGroupWidget(
	QWidget* parent
)
	: EditWidget(
		parent
	),
	mainUi(
		new Ui::EditGroupWidgetMain()
	),
	editGroupWidgetMain(
		new QWidget()
	),
	editGroupWidgetIcons(
		new EditWidgetIcons()
	),
	editWidgetProperties(
		new EditWidgetProperties()
	),
	group(
		nullptr
	),
	database(
		nullptr
	)
{
	this->mainUi->setupUi(
		this->editGroupWidgetMain
	);
	this->add(
		this->tr(
			"Group"
		),
		this->editGroupWidgetMain
	);
	this->add(
		this->tr(
			"Icon"
		),
		this->editGroupWidgetIcons
	);
	this->add(
		this->tr(
			"Properties"
		),
		this->editWidgetProperties
	);
	this->connect(
		this->mainUi->expireCheck,
		&QCheckBox::toggled,
		this->mainUi->expireDatePicker,
		&QDateTimeEdit::setEnabled
	);
	this->connect(
		this->mainUi->autoTypeSequenceCustomRadio,
		&QRadioButton::toggled,
		this->mainUi->autoTypeSequenceCustomEdit,
		&QLineEdit::setEnabled
	);
	this->connect(
		this,
		&EditGroupWidget::sig_accepted,
		this,
		&EditGroupWidget::do_save
	);
	this->connect(
		this,
		&EditGroupWidget::sig_rejected,
		this,
		&EditGroupWidget::do_cancel
	);
}

EditGroupWidget::~EditGroupWidget()
{
}

void EditGroupWidget::loadGroup(
	Group* group,
	const bool create,
	Database* database
)
{
	this->group = group;
	this->database = database;
	if(create)
	{
		this->setHeadline(
			this->tr(
				"Add group"
			)
		);
	}
	else
	{
		this->setHeadline(
			this->tr(
				"Edit group"
			)
		);
	}
	if(this->group->getParentGroup())
	{
		this->addTriStateItems(
			this->mainUi->searchComboBox,
			this->group->getParentGroup()->isResolveSearchingEnabled()
		);
		this->addTriStateItems(
			this->mainUi->autotypeComboBox,
			this->group->getParentGroup()->isResolveAutoTypeEnabled()
		);
	}
	else
	{
		this->addTriStateItems(
			this->mainUi->searchComboBox,
			true
		);
		this->addTriStateItems(
			this->mainUi->autotypeComboBox,
			true
		);
	}
	this->mainUi->editName->setText(
		this->group->getName()
	);
	this->mainUi->editNotes->setPlainText(
		this->group->getNotes()
	);
	this->mainUi->expireCheck->setChecked(
		this->group->getTimeInfo().getExpires()
	);
	this->mainUi->expireDatePicker->setDateTime(
		this->group->getTimeInfo().getExpiryTime().toLocalTime()
	);
	this->mainUi->searchComboBox->setCurrentIndex(
		this->indexFromTriState(
			this->group->isSearchingEnabled()
		)
	);
	this->mainUi->autotypeComboBox->setCurrentIndex(
		this->indexFromTriState(
			this->group->isAutoTypeEnabled()
		)
	);
	if(group->getDefaultAutoTypeSequence().isEmpty())
	{
		this->mainUi->autoTypeSequenceInherit->setChecked(
			true
		);
	}
	else
	{
		this->mainUi->autoTypeSequenceCustomRadio->setChecked(
			true
		);
	}
	this->mainUi->autoTypeSequenceCustomEdit->setText(
		this->group->getDefaultAutoTypeSequence()
	);
	IconStruct iconStruct_;
	iconStruct_.uuid = this->group->getIconUUID();
	iconStruct_.number = this->group->getIconNumber();
	this->editGroupWidgetIcons->load(
		this->group->getUUID(),
		this->database,
		iconStruct_
	);
	this->editWidgetProperties->setFields(
		this->group->getTimeInfo(),
		this->group->getUUID()
	);
	this->setCurrentRow(
		0
	);
	this->mainUi->editName->setFocus();
}

void EditGroupWidget::do_save()
{
	this->group->setName(
		this->mainUi->editName->text()
	);
	this->group->setNotes(
		this->mainUi->editNotes->toPlainText()
	);
	this->group->setExpires(
		this->mainUi->expireCheck->isChecked()
	);
	this->group->setExpiryTime(
		this->mainUi->expireDatePicker->dateTime().toUTC()
	);
	this->group->setSearchingEnabled(
		this->triStateFromIndex(
			this->mainUi->searchComboBox->currentIndex()
		)
	);
	this->group->setAutoTypeEnabled(
		this->triStateFromIndex(
			this->mainUi->autotypeComboBox->currentIndex()
		)
	);
	if(this->mainUi->autoTypeSequenceInherit->isChecked())
	{
		this->group->setDefaultAutoTypeSequence(
			QString()
		);
	}
	else
	{
		this->group->setDefaultAutoTypeSequence(
			this->mainUi->autoTypeSequenceCustomEdit->text()
		);
	}
	if(const IconStruct iconStruct_ = this->editGroupWidgetIcons->state();
		iconStruct_.number < 0)
	{
		this->group->setIcon(
			Group::DefaultIconNumber
		);
	}
	else if(iconStruct_.uuid.isNull())
	{
		this->group->setIcon(
			iconStruct_.number
		);
	}
	else
	{
		this->group->setIcon(
			iconStruct_.uuid
		);
	}
	this->clear();
	 this->sig_editFinished(
		true
	);
}

void EditGroupWidget::do_cancel()
{
	if(!this->group->getIconUUID().isNull() && !this->database->getMetadata()->
		containsCustomIcon(
			this->group->getIconUUID()
		))
	{
		this->group->setIcon(
			Entry::DefaultIconNumber
		);
	}
	this->clear();
	 this->sig_editFinished(
		false
	);
}

void EditGroupWidget::clear()
{
	this->group = nullptr;
	this->database = nullptr;
	this->editGroupWidgetIcons->reset();
}

void EditGroupWidget::addTriStateItems(
	QComboBox* comboBox,
	const bool inheritValue
)
{
	QString inheritDefaultString_;
	if(inheritValue)
	{
		inheritDefaultString_ = tr(
			"Enable"
		);
	}
	else
	{
		inheritDefaultString_ = tr(
			"Disable"
		);
	}
	comboBox->clear();
	comboBox->addItem(
		tr(
			"Inherit from parent group (%1)"
		).arg(
			inheritDefaultString_
		)
	);
	comboBox->addItem(
		tr(
			"Enable"
		)
	);
	comboBox->addItem(
		tr(
			"Disable"
		)
	);
}

int EditGroupWidget::indexFromTriState(
	const Group::TriState triState
)
{
	switch(triState)
	{
		case Group::Inherit:
			return 0;
		case Group::Enable:
			return 1;
		case Group::Disable:
			return 2;
		default:
			return 0;
	}
}

Group::TriState EditGroupWidget::triStateFromIndex(
	const int index
)
{
	switch(index)
	{
		case 0:
			return Group::Inherit;
		case 1:
			return Group::Enable;
		case 2:
			return Group::Disable;
		default:
			return Group::Inherit;
	}
}
