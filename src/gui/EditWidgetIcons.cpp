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
#include "EditWidgetIcons.h"
#include <QFileDialog>
#include <QImageReader>
#include "ui_EditWidgetIcons.h"
#include "core/Global.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "core/Tools.h"
#include "gui/IconModels.h"
#include "gui/MessageBox.h"

IconStruct::IconStruct()
	: uuid(
		UUID()
	),
	number(
		0
	)
{
}

EditWidgetIcons::EditWidgetIcons(
	QWidget* parent
)
	: QWidget(
		parent
	),
	ui(
		new Ui::EditWidgetIcons()
	),
	database(
		nullptr
	),
	defaultIconModel(
		new DefaultIconModel(
			this
		)
	),
	customIconModel(
		new CustomIconModel(
			this
		)
	)
{
	this->ui->setupUi(
		this
	);
	this->ui->defaultIconsView->setModel(
		this->defaultIconModel
	);
	this->ui->customIconsView->setModel(
		this->customIconModel
	);
	this->connect(
		this->ui->defaultIconsView,
		&QListView::clicked,
		this,
		&EditWidgetIcons::do_updateRadioButtonDefaultIcons
	);
	this->connect(
		this->ui->customIconsView,
		&QListView::clicked,
		this,
		&EditWidgetIcons::do_updateRadioButtonCustomIcons
	);
	this->connect(
		this->ui->defaultIconsRadio,
		&QRadioButton::toggled,
		this,
		&EditWidgetIcons::do_updateWidgetsDefaultIcons
	);
	this->connect(
		this->ui->customIconsRadio,
		&QRadioButton::toggled,
		this,
		&EditWidgetIcons::do_updateWidgetsCustomIcons
	);
	this->connect(
		this->ui->addButton,
		&QRadioButton::clicked,
		this,
		&EditWidgetIcons::do_addCustomIcon
	);
	this->connect(
		this->ui->deleteButton,
		&QRadioButton::clicked,
		this,
		&EditWidgetIcons::do_removeCustomIcon
	);
}

EditWidgetIcons::~EditWidgetIcons()
{
}

IconStruct EditWidgetIcons::state() const
{
	IconStruct iconStruct_;
	if(this->ui->defaultIconsRadio->isChecked())
	{
		if(const QModelIndex index_ = this->ui->defaultIconsView->currentIndex()
			;
			index_.isValid())
		{
			iconStruct_.number = index_.row();
		}
	}
	else
	{
		if(const QModelIndex index_ = this->ui->customIconsView->currentIndex();
			index_.isValid())
		{
			iconStruct_.uuid = this->customIconModel->uuidFromIndex(
				this->ui->customIconsView->currentIndex()
			);
		}
		else
		{
			iconStruct_.number = -1;
		}
	}
	return iconStruct_;
}

void EditWidgetIcons::reset()
{
	this->database = nullptr;
	this->currentUUID = UUID();
}

void EditWidgetIcons::load(
	const UUID &currentUuid,
	Database* database,
	const IconStruct &iconStruct
)
{
	this->database = database;
	this->currentUUID = currentUuid;
	this->customIconModel->setIcons(
		database->getMetadata()->customIconsScaledPixmaps(),
		database->getMetadata()->getCustomIconsOrder()
	);
	if(const UUID iconUuid_ = iconStruct.uuid;
		iconUuid_.isNull())
	{
		const int iconNumber_ = iconStruct.number;
		this->ui->defaultIconsView->setCurrentIndex(
			this->defaultIconModel->index(
				iconNumber_,
				0
			)
		);
		this->ui->defaultIconsRadio->setChecked(
			true
		);
	}
	else
	{
		if(const QModelIndex index_ = this->customIconModel->indexFromUuid(
				iconUuid_
			);
			index_.isValid())
		{
			this->ui->customIconsView->setCurrentIndex(
				index_
			);
			this->ui->customIconsRadio->setChecked(
				true
			);
		}
		else
		{
			this->ui->defaultIconsView->setCurrentIndex(
				this->defaultIconModel->index(
					0,
					0
				)
			);
			this->ui->defaultIconsRadio->setChecked(
				true
			);
		}
	}
}

void EditWidgetIcons::do_addCustomIcon()
{
	if(this->database)
	{
		const QString filter_ = QString(
			"%1 (%2);;%3 (*)"
		).arg(
			this->tr(
				"Images"
			),
			Tools::getImageReaderFilter(),
			this->tr(
				"All files"
			)
		);
		if(const QString filename_ = QFileDialog::getOpenFileName(
				this,
				this->tr(
					"Select Image"
				),
				"",
				filter_
			);
			!filename_.isEmpty())
		{
			QImageReader imageReader_(
				filename_
			);
			// detect from content, otherwise reading fails if file extension is wrong
			imageReader_.setDecideFormatFromContent(
				true
			);
			if(const QImage image_ = imageReader_.read();
				!image_.isNull())
			{
				const UUID uuid_ = UUID::random();
				this->database->getMetadata()->addCustomIconScaled(
					uuid_,
					image_
				);
				this->customIconModel->setIcons(
					this->database->getMetadata()->customIconsScaledPixmaps(),
					this->database->getMetadata()->getCustomIconsOrder()
				);
				const QModelIndex index_ = this->customIconModel->indexFromUuid(
					uuid_
				);
				this->ui->customIconsView->setCurrentIndex(
					index_
				);
			}
			else
			{
				MessageBox::critical(
					this,
					this->tr(
						"Error"
					),
					this->tr(
						"Can't read icon:"
					).append(
						"\n"
					).append(
						imageReader_.errorString()
					)
				);
			}
		}
	}
}

void EditWidgetIcons::do_removeCustomIcon()
{
	if(this->database)
	{
		if(const QModelIndex index_ = this->ui->customIconsView->currentIndex();
			index_.isValid())
		{
			const UUID iconUuid_ = this->customIconModel->uuidFromIndex(
				index_
			);
			auto iconUsedCount_ = 0;
			const QList<Entry*> allEntries_ = this->database->getRootGroup()->
				getEntriesRecursive(
					true
				);
			QList<Entry*> historyEntriesWithSameIcon_;
			for(Entry* entry_: allEntries_)
			{
				const bool isHistoryEntry_ = !entry_->getGroup();
				if(iconUuid_ == entry_->getIconUUID())
				{
					if(isHistoryEntry_)
					{
						historyEntriesWithSameIcon_ << entry_;
					}
					else if(this->currentUUID != entry_->getUUID())
					{
						iconUsedCount_++;
					}
				}
			}
			const QList<Group*> allGroups_ = this->database->getRootGroup()->
				getGroupsRecursive(
					true
				);
			for(const Group* group_: allGroups_)
			{
				if(iconUuid_ == group_->getIconUUID() && this->currentUUID !=
					group_->getUUID())
				{
					iconUsedCount_++;
				}
			}
			if(iconUsedCount_ == 0)
			{
				for(Entry* entry_: asConst(
						historyEntriesWithSameIcon_
					))
				{
					entry_->setUpdateTimeinfo(
						false
					);
					entry_->setIcon(
						0
					);
					entry_->setUpdateTimeinfo(
						true
					);
				}
				this->database->getMetadata()->removeCustomIcon(
					iconUuid_
				);
				this->customIconModel->setIcons(
					this->database->getMetadata()->customIconsScaledPixmaps(),
					this->database->getMetadata()->getCustomIconsOrder()
				);
				if(this->customIconModel->rowCount() > 0)
				{
					this->ui->customIconsView->setCurrentIndex(
						this->customIconModel->index(
							0,
							0
						)
					);
				}
				else
				{
					this->do_updateRadioButtonDefaultIcons();
				}
			}
			else
			{
				MessageBox::information(
					this,
					this->tr(
						"Can't delete icon!"
					),
					this->tr(
						"Can't delete icon. Still used by %n item(s).",
						nullptr,
						iconUsedCount_
					)
				);
			}
		}
	}
}

void EditWidgetIcons::do_updateWidgetsDefaultIcons(
	const bool checked
) const
{
	if(checked)
	{
		if(const QModelIndex index_ = this->ui->defaultIconsView->currentIndex()
			;
			!index_.isValid())
		{
			this->ui->defaultIconsView->setCurrentIndex(
				this->defaultIconModel->index(
					0,
					0
				)
			);
		}
		else
		{
			this->ui->defaultIconsView->setCurrentIndex(
				index_
			);
		}
		this->ui->customIconsView->selectionModel()->clearSelection();
		this->ui->addButton->setEnabled(
			false
		);
		this->ui->deleteButton->setEnabled(
			false
		);
	}
}

void EditWidgetIcons::do_updateWidgetsCustomIcons(
	const bool checked
) const
{
	if(checked)
	{
		if(const QModelIndex index_ = this->ui->customIconsView->currentIndex();
			!index_.isValid())
		{
			this->ui->customIconsView->setCurrentIndex(
				this->customIconModel->index(
					0,
					0
				)
			);
		}
		else
		{
			this->ui->customIconsView->setCurrentIndex(
				index_
			);
		}
		this->ui->defaultIconsView->selectionModel()->clearSelection();
		this->ui->addButton->setEnabled(
			true
		);
		this->ui->deleteButton->setEnabled(
			true
		);
	}
}

void EditWidgetIcons::do_updateRadioButtonDefaultIcons() const
{
	this->ui->defaultIconsRadio->setChecked(
		true
	);
}

void EditWidgetIcons::do_updateRadioButtonCustomIcons() const
{
	this->ui->customIconsRadio->setChecked(
		true
	);
}
