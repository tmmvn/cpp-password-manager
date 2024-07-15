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
#include "EditEntryWidget.h"
#include <QButtonGroup>
#include <QDesktopServices>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QTemporaryFile>
#include "ui_EditEntryWidgetAdvanced.h"
#include "ui_EditEntryWidgetHistory.h"
#include "ui_EditEntryWidgetMain.h"
#include "core/Config.h"
#include "core/Database.h"
#include "core/Entry.h"
#include "core/FilePath.h"
#include "core/Metadata.h"
#include "core/TimeDelta.h"
#include "core/Tools.h"
#include "gui/EditWidgetIcons.h"
#include "gui/EditWidgetProperties.h"
#include "gui/FileDialog.h"
#include "gui/MessageBox.h"
#include "gui/entry/EntryAttachmentsModel.h"
#include "gui/entry/EntryAttributesModel.h"
#include "gui/entry/EntryHistoryModel.h"

EditEntryWidget::EditEntryWidget(
	QWidget* parent
)
	: EditWidget(
		parent
	),
	entry(
		nullptr
	),
	mainUi(
		new Ui::EditEntryWidgetMain()
	),
	advancedUi(
		new Ui::EditEntryWidgetAdvanced()
	),
	historyUi(
		new Ui::EditEntryWidgetHistory()
	),
	mainWidget(
		new QWidget()
	),
	advancedWidget(
		new QWidget()
	),
	iconsWidget(
		new EditWidgetIcons()
	),
	editWidgetProperties(
		new EditWidgetProperties()
	),
	historyWidget(
		new QWidget()
	),
	entryAttachments(
		new EntryAttachments(
			this
		)
	),
	attachmentsModel(
		new EntryAttachmentsModel(
			advancedWidget
		)
	),
	entryAttributes(
		new EntryAttributes(
			this
		)
	),
	attributesModel(
		new EntryAttributesModel(
			advancedWidget
		)
	),
	historyModel(
		new EntryHistoryModel(
			this
		)
	),
	sortModel(
		new QSortFilterProxyModel(
			this
		)
	)
{
	this->setupMain();
	this->setupAdvanced();
	this->setupIcon();
	this->setupProperties();
	this->setupHistory();
	this->connect(
		this,
		&EditEntryWidget::sig_accepted,
		this,
		&EditEntryWidget::do_saveEntry
	);
	this->connect(
		this,
		&EditEntryWidget::sig_rejected,
		this,
		&EditEntryWidget::do_cancel
	);
}

EditEntryWidget::~EditEntryWidget()
{
}

void EditEntryWidget::setupMain()
{
	this->mainUi->setupUi(
		this->mainWidget
	);
	this->add(
		this->tr(
			"Entry"
		),
		this->mainWidget
	);
	this->mainUi->togglePasswordButton->setIcon(
		FilePath::getInstance()->getOnOffIcon(
			"actions",
			"password-show"
		)
	);
	this->connect(
		this->mainUi->togglePasswordButton,
		&QToolButton::toggled,
		this->mainUi->passwordEdit,
		&PasswordEdit::do_setShowPassword
	);
	this->connect(
		this->mainUi->tooglePasswordGeneratorButton,
		&QToolButton::toggled,
		this,
		&EditEntryWidget::do_togglePasswordGeneratorButton
	);
	this->connect(
		this->mainUi->expireCheck,
		&QCheckBox::toggled,
		this->mainUi->expireDatePicker,
		&QDateTimeEdit::setEnabled
	);
	this->mainUi->passwordRepeatEdit->enableVerifyMode(
		this->mainUi->passwordEdit
	);
	this->connect(
		this->mainUi->passwordGenerator,
		&PasswordGeneratorWidget::sig_newPassword,
		this,
		&EditEntryWidget::do_setGeneratedPassword
	);
	this->mainUi->expirePresets->setMenu(
		this->createPresetsMenu()
	);
	this->connect(
		this->mainUi->expirePresets->menu(),
		&QMenu::triggered,
		this,
		&EditEntryWidget::do_useExpiryPreset
	);
	this->mainUi->passwordGenerator->hide();
	this->mainUi->passwordGenerator->reset();
}

void EditEntryWidget::setupAdvanced() const
{
	this->advancedUi->setupUi(
		this->advancedWidget
	);
	this->add(
		this->tr(
			"Advanced"
		),
		this->advancedWidget
	);
	this->attachmentsModel->setEntryAttachments(
		this->entryAttachments
	);
	this->advancedUi->attachmentsView->setModel(
		this->attachmentsModel
	);
	this->connect(
		this->advancedUi->attachmentsView->selectionModel(),
		&QItemSelectionModel::currentChanged,
		this,
		&EditEntryWidget::do_updateAttachmentButtonsEnabled
	);
	this->connect(
		this->advancedUi->attachmentsView,
		&QListView::doubleClicked,
		this,
		&EditEntryWidget::do_openAttachment
	);
	this->connect(
		this->advancedUi->saveAttachmentButton,
		&QPushButton::clicked,
		this,
		&EditEntryWidget::do_saveCurrentAttachment
	);
	this->connect(
		this->advancedUi->openAttachmentButton,
		&QPushButton::clicked,
		this,
		&EditEntryWidget::do_openCurrentAttachment
	);
	this->connect(
		this->advancedUi->addAttachmentButton,
		&QPushButton::clicked,
		this,
		&EditEntryWidget::do_insertAttachment
	);
	this->connect(
		this->advancedUi->removeAttachmentButton,
		&QPushButton::clicked,
		this,
		&EditEntryWidget::do_removeCurrentAttachment
	);
	this->attributesModel->setEntryAttributes(
		this->entryAttributes
	);
	this->advancedUi->attributesView->setModel(
		this->attributesModel
	);
	this->connect(
		this->advancedUi->addAttributeButton,
		&QPushButton::clicked,
		this,
		&EditEntryWidget::do_insertAttribute
	);
	this->connect(
		this->advancedUi->editAttributeButton,
		&QPushButton::clicked,
		this,
		&EditEntryWidget::do_editCurrentAttribute
	);
	this->connect(
		this->advancedUi->removeAttributeButton,
		&QPushButton::clicked,
		this,
		&EditEntryWidget::do_removeCurrentAttribute
	);
	this->connect(
		this->advancedUi->attributesView->selectionModel(),
		&QItemSelectionModel::currentChanged,
		this,
		&EditEntryWidget::do_updateCurrentAttribute
	);
}

void EditEntryWidget::setupIcon() const
{
	this->add(
		this->tr(
			"Icon"
		),
		this->iconsWidget
	);
}

void EditEntryWidget::setupProperties() const
{
	this->add(
		this->tr(
			"Properties"
		),
		this->editWidgetProperties
	);
}

void EditEntryWidget::setupHistory() const
{
	this->historyUi->setupUi(
		this->historyWidget
	);
	this->add(
		this->tr(
			"History"
		),
		this->historyWidget
	);
	this->sortModel->setSourceModel(
		this->historyModel
	);
	this->sortModel->setDynamicSortFilter(
		true
	);
	this->sortModel->setSortLocaleAware(
		true
	);
	this->sortModel->setSortCaseSensitivity(
		Qt::CaseInsensitive
	);
	this->sortModel->setSortRole(
		Qt::UserRole
	);
	this->historyUi->historyView->setModel(
		this->sortModel
	);
	this->historyUi->historyView->setRootIsDecorated(
		false
	);
	this->connect(
		this->historyUi->historyView,
		&QTreeView::activated,
		this,
		&EditEntryWidget::do_histEntryActivated
	);
	this->connect(
		this->historyUi->historyView->selectionModel(),
		&QItemSelectionModel::currentChanged,
		this,
		&EditEntryWidget::do_updateHistoryButtons
	);
	this->connect(
		this->historyUi->showButton,
		&QPushButton::clicked,
		this,
		&EditEntryWidget::do_showHistoryEntry
	);
	this->connect(
		this->historyUi->restoreButton,
		&QPushButton::clicked,
		this,
		&EditEntryWidget::do_restoreHistoryEntry
	);
	this->connect(
		this->historyUi->deleteButton,
		&QPushButton::clicked,
		this,
		&EditEntryWidget::do_deleteHistoryEntry
	);
	this->connect(
		this->historyUi->deleteAllButton,
		&QPushButton::clicked,
		this,
		&EditEntryWidget::do_deleteAllHistoryEntries
	);
}

void EditEntryWidget::do_emitHistoryEntryActivated(
	const QModelIndex &index
)
{
	if(!this->history)
	{
		return;
	}
	Entry* entry_ = this->historyModel->entryFromIndex(
		index
	);
	this->sig_historyEntryActivated(
		entry_
	);
}

void EditEntryWidget::do_histEntryActivated(
	const QModelIndex &index
)
{
	if(!this->history)
	{
		return;
	}
	if(const QModelIndex indexMapped_ = this->sortModel->mapToSource(
			index
		);
		indexMapped_.isValid())
	{
		this->do_emitHistoryEntryActivated(
			indexMapped_
		);
	}
}

void EditEntryWidget::do_updateHistoryButtons(
	const QModelIndex &current,
	const QModelIndex &previous
) const
{
	Q_UNUSED(
		previous
	);
	if(current.isValid())
	{
		this->historyUi->showButton->setEnabled(
			true
		);
		this->historyUi->restoreButton->setEnabled(
			true
		);
		this->historyUi->deleteButton->setEnabled(
			true
		);
	}
	else
	{
		this->historyUi->showButton->setEnabled(
			false
		);
		this->historyUi->restoreButton->setEnabled(
			false
		);
		this->historyUi->deleteButton->setEnabled(
			false
		);
	}
}

void EditEntryWidget::do_useExpiryPreset(
	const QAction* action
) const
{
	this->mainUi->expireCheck->setChecked(
		true
	);
	const auto delta_ = action->data().value<TimeDelta>();
	const QDateTime now_ = QDateTime::currentDateTime();
	const QDateTime expiryDateTime_ = now_ + delta_;
	this->mainUi->expireDatePicker->setDateTime(
		expiryDateTime_
	);
}

void EditEntryWidget::do_updateAttachmentButtonsEnabled(
	const QModelIndex &current
) const
{
	const bool enable_ = current.isValid();
	this->advancedUi->saveAttachmentButton->setEnabled(
		enable_
	);
	this->advancedUi->openAttachmentButton->setEnabled(
		enable_
	);
	this->advancedUi->removeAttachmentButton->setEnabled(
		enable_ && !this->history
	);
}

QString EditEntryWidget::entryTitle() const
{
	if(this->entry)
	{
		return this->entry->getTitle();
	}
	return QString();
}

void EditEntryWidget::loadEntry(
	Entry* entry,
	const bool create,
	const bool history,
	const QString &parentName,
	Database* database
)
{
	this->entry = entry;
	this->database = database;
	this->create = create;
	this->history = history;
	if(this->history)
	{
		this->setHeadline(
			QString(
				"%1 > %2"
			).arg(
				parentName,
				this->tr(
					"Entry history"
				)
			)
		);
	}
	else
	{
		if(create)
		{
			this->setHeadline(
				QString(
					"%1 > %2"
				).arg(
					parentName,
					this->tr(
						"Add entry"
					)
				)
			);
		}
		else
		{
			this->setHeadline(
				QString(
					"%1 > %2 > %3"
				).arg(
					parentName,
					entry->getTitle(),
					this->tr(
						"Edit entry"
					)
				)
			);
		}
	}
	this->setForms(
		entry
	);
	this->setReadOnly(
		this->history
	);
	this->setCurrentRow(
		0
	);
	this->setRowHidden(
		this->historyWidget,
		this->history
	);
}

void EditEntryWidget::setForms(
	const Entry* entry,
	const bool restore
) const
{
	this->mainUi->titleEdit->setReadOnly(
		this->history
	);
	this->mainUi->usernameEdit->setReadOnly(
		this->history
	);
	this->mainUi->urlEdit->setReadOnly(
		this->history
	);
	this->mainUi->passwordEdit->setReadOnly(
		this->history
	);
	this->mainUi->passwordRepeatEdit->setReadOnly(
		this->history
	);
	this->mainUi->expireCheck->setEnabled(
		!this->history
	);
	this->mainUi->expireDatePicker->setReadOnly(
		this->history
	);
	this->mainUi->notesEdit->setReadOnly(
		this->history
	);
	this->mainUi->tooglePasswordGeneratorButton->setChecked(
		false
	);
	this->mainUi->tooglePasswordGeneratorButton->setDisabled(
		this->history
	);
	this->mainUi->passwordGenerator->reset();
	this->advancedUi->addAttachmentButton->setEnabled(
		!this->history
	);
	this->do_updateAttachmentButtonsEnabled(
		this->advancedUi->attachmentsView->currentIndex()
	);
	this->advancedUi->addAttributeButton->setEnabled(
		!this->history
	);
	this->advancedUi->editAttributeButton->setEnabled(
		false
	);
	this->advancedUi->removeAttributeButton->setEnabled(
		false
	);
	this->advancedUi->attributesEdit->setReadOnly(
		this->history
	);
	QAbstractItemView::EditTriggers editTriggers_;
	if(this->history)
	{
		editTriggers_ = QAbstractItemView::NoEditTriggers;
	}
	else
	{
		editTriggers_ = QAbstractItemView::DoubleClicked;
	}
	this->advancedUi->attributesView->setEditTriggers(
		editTriggers_
	);
	this->iconsWidget->setEnabled(
		!this->history
	);
	this->historyWidget->setEnabled(
		!this->history
	);
	this->mainUi->titleEdit->setText(
		entry->getTitle()
	);
	this->mainUi->usernameEdit->setText(
		entry->getUsername()
	);
	this->mainUi->urlEdit->setText(
		entry->getURL()
	);
	this->mainUi->passwordEdit->setText(
		entry->getPassword()
	);
	this->mainUi->passwordRepeatEdit->setText(
		entry->getPassword()
	);
	this->mainUi->expireCheck->setChecked(
		entry->getTimeInfo().getExpires()
	);
	this->mainUi->expireDatePicker->setDateTime(
		entry->getTimeInfo().getExpiryTime().toLocalTime()
	);
	this->mainUi->expirePresets->setEnabled(
		!this->history
	);
	this->mainUi->togglePasswordButton->setChecked(
		Config::getInstance()->get(
			"security/passwordscleartext"
		).toBool()
	);
	this->mainUi->notesEdit->setPlainText(
		entry->getNotes()
	);
	this->entryAttachments->copyDataFrom(
		entry->getAttachments()
	);
	this->entryAttributes->copyCustomKeysFrom(
		entry->getAttributes()
	);
	if(this->attributesModel->rowCount() != 0)
	{
		this->advancedUi->attributesView->setCurrentIndex(
			this->attributesModel->index(
				0,
				0
			)
		);
	}
	else
	{
		this->advancedUi->attributesEdit->setPlainText(
			""
		);
		this->advancedUi->attributesEdit->setEnabled(
			false
		);
	}
	IconStruct iconStruct_;
	iconStruct_.uuid = entry->getIconUUID();
	iconStruct_.number = entry->getIconNumber();
	this->iconsWidget->load(
		entry->getUUID(),
		this->database,
		iconStruct_
	);
	this->editWidgetProperties->setFields(
		entry->getTimeInfo(),
		entry->getUUID()
	);
	if(!this->history && !restore)
	{
		this->historyModel->setEntries(
			entry->getHistoryItems()
		);
		this->historyUi->historyView->sortByColumn(
			0,
			Qt::DescendingOrder
		);
	}
	if(this->historyModel->rowCount() > 0)
	{
		this->historyUi->deleteAllButton->setEnabled(
			true
		);
	}
	else
	{
		this->historyUi->deleteAllButton->setEnabled(
			false
		);
	}
	this->do_updateHistoryButtons(
		this->historyUi->historyView->currentIndex(),
		QModelIndex()
	);
	this->mainUi->titleEdit->setFocus();
}

void EditEntryWidget::do_saveEntry()
{
	if(this->history)
	{
		this->clear();
		this->sig_editFinished(
			false
		);
		return;
	}
	if(!this->passwordsEqual())
	{
		MessageBox::warning(
			this,
			this->tr(
				"Error"
			),
			this->tr(
				"Different passwords supplied."
			)
		);
		return;
	}
	if(this->advancedUi->attributesView->currentIndex().isValid())
	{
		const QString key_ = this->attributesModel->keyByIndex(
			this->advancedUi->attributesView->currentIndex()
		);
		this->entryAttributes->set(
			key_,
			this->advancedUi->attributesEdit->toPlainText(),
			this->entryAttributes->isProtected(
				key_
			)
		);
	}
	this->currentAttribute = QPersistentModelIndex();
	// must stand before beginUpdate()
	// we don't want to create a new history item, if only the history has changed
	this->entry->removeHistoryItems(
		this->historyModel->deletedEntries()
	);
	if(!this->create)
	{
		this->entry->beginUpdate();
	}
	this->updateEntryData(
		this->entry
	);
	if(!this->create)
	{
		this->entry->endUpdate();
	}
	this->clear();
	this->sig_editFinished(
		true
	);
}

void EditEntryWidget::updateEntryData(
	Entry* entry
) const
{
	entry->setTitle(
		this->mainUi->titleEdit->text()
	);
	entry->setUsername(
		this->mainUi->usernameEdit->text()
	);
	entry->setURL(
		this->mainUi->urlEdit->text()
	);
	entry->setPassword(
		this->mainUi->passwordEdit->text()
	);
	entry->setExpires(
		this->mainUi->expireCheck->isChecked()
	);
	entry->setExpiryTime(
		this->mainUi->expireDatePicker->dateTime().toUTC()
	);
	entry->setNotes(
		this->mainUi->notesEdit->toPlainText()
	);
	entry->getAttributes()->copyCustomKeysFrom(
		this->entryAttributes
	);
	entry->getAttachments()->copyDataFrom(
		this->entryAttachments
	);
	if(const IconStruct iconStruct_ = this->iconsWidget->state();
		iconStruct_.number < 0)
	{
		entry->setIcon(
			Entry::DefaultIconNumber
		);
	}
	else if(iconStruct_.uuid.isNull())
	{
		entry->setIcon(
			iconStruct_.number
		);
	}
	else
	{
		entry->setIcon(
			iconStruct_.uuid
		);
	}
}

void EditEntryWidget::do_cancel()
{
	if(this->history)
	{
		this->clear();
		this->sig_editFinished(
			false
		);
		return;
	}
	if(!this->entry->getIconUUID().isNull() && !this->database->getMetadata()->
		containsCustomIcon(
			this->entry->getIconUUID()
		))
	{
		this->entry->setIcon(
			Entry::DefaultIconNumber
		);
	}
	this->clear();
	this->sig_editFinished(
		false
	);
}

void EditEntryWidget::clear()
{
	this->entry = nullptr;
	this->database = nullptr;
	this->entryAttributes->clear();
	this->entryAttachments->clear();
	this->historyModel->clear();
	this->iconsWidget->reset();
}

bool EditEntryWidget::hasBeenModified() const
{
	// entry has been modified if a history item is to be deleted
	if(!this->historyModel->deletedEntries().isEmpty())
	{
		return true;
	}
	// check if updating the entry would modify it
	const QScopedPointer entry_(
		new Entry()
	);
	entry_->copyDataFrom(
		this->entry
	);
	entry_->beginUpdate();
	this->updateEntryData(
		entry_.data()
	);
	return entry_->endUpdate();
}

void EditEntryWidget::do_togglePasswordGeneratorButton(
	const bool checked
) const
{
	this->mainUi->passwordGenerator->regeneratePassword();
	this->mainUi->passwordGenerator->setVisible(
		checked
	);
}

bool EditEntryWidget::passwordsEqual() const
{
	return this->mainUi->passwordEdit->text() == this->mainUi->
		passwordRepeatEdit->text();
}

void EditEntryWidget::do_setGeneratedPassword(
	const QString &password
) const
{
	this->mainUi->passwordEdit->setText(
		password
	);
	this->mainUi->passwordRepeatEdit->setText(
		password
	);
	this->mainUi->tooglePasswordGeneratorButton->setChecked(
		false
	);
}

void EditEntryWidget::do_insertAttribute() const
{
	if(this->history)
	{
		return;
	}
	QString name_ = this->tr(
		"New attribute"
	);
	auto i_ = 1;
	while(this->entryAttributes->getKeys().contains(
		name_
	))
	{
		name_ = QString(
			"%1 %2"
		).arg(
			this->tr(
				"New attribute"
			)
		).arg(
			i_
		);
		i_++;
	}
	this->entryAttributes->set(
		name_,
		""
	);
	const QModelIndex index_ = this->attributesModel->indexByKey(
		name_
	);
	this->advancedUi->attributesView->setCurrentIndex(
		index_
	);
	this->advancedUi->attributesView->edit(
		index_
	);
}

void EditEntryWidget::do_editCurrentAttribute() const
{
	if(this->history)
	{
		return;
	}
	if(const QModelIndex index = this->advancedUi->attributesView->
			currentIndex();
		index.isValid())
	{
		this->advancedUi->attributesView->edit(
			index
		);
	}
}

void EditEntryWidget::do_removeCurrentAttribute() const
{
	if(this->history)
	{
		return;
	}
	if(const QModelIndex index_ = this->advancedUi->attributesView->
			currentIndex();
		index_.isValid())
	{
		this->entryAttributes->remove(
			this->attributesModel->keyByIndex(
				index_
			)
		);
	}
}

void EditEntryWidget::do_updateCurrentAttribute()
{
	const QModelIndex newIndex_ = this->advancedUi->attributesView->
		currentIndex();
	if(this->history)
	{
		if(newIndex_.isValid())
		{
			const QString key_ = this->attributesModel->keyByIndex(
				newIndex_
			);
			this->advancedUi->attributesEdit->setPlainText(
				this->entryAttributes->getValue(
					key_
				)
			);
			this->advancedUi->attributesEdit->setEnabled(
				true
			);
		}
		else
		{
			this->advancedUi->attributesEdit->setPlainText(
				""
			);
			this->advancedUi->attributesEdit->setEnabled(
				false
			);
		}
	}
	else
	{
		if(this->currentAttribute != newIndex_)
		{
			if(this->currentAttribute.isValid())
			{
				const QString key_ = this->attributesModel->keyByIndex(
					this->currentAttribute
				);
				this->entryAttributes->set(
					key_,
					this->advancedUi->attributesEdit->toPlainText(),
					this->entryAttributes->isProtected(
						key_
					)
				);
			}
			if(newIndex_.isValid())
			{
				const QString key_ = this->attributesModel->keyByIndex(
					newIndex_
				);
				this->advancedUi->attributesEdit->setPlainText(
					this->entryAttributes->getValue(
						key_
					)
				);
				this->advancedUi->attributesEdit->setEnabled(
					true
				);
			}
			else
			{
				this->advancedUi->attributesEdit->setPlainText(
					""
				);
				this->advancedUi->attributesEdit->setEnabled(
					false
				);
			}
			this->advancedUi->editAttributeButton->setEnabled(
				newIndex_.isValid()
			);
			this->advancedUi->removeAttributeButton->setEnabled(
				newIndex_.isValid()
			);
			this->currentAttribute = newIndex_;
		}
	}
}

void EditEntryWidget::do_insertAttachment()
{
	if(this->history)
	{
		return;
	}
	QString defaultDir_ = Config::getInstance()->get(
		"LastAttachmentDir"
	).toString();
	if(defaultDir_.isEmpty() || !QDir(
		defaultDir_
	).exists())
	{
		defaultDir_ = QStandardPaths::standardLocations(
			QStandardPaths::DocumentsLocation
		).value(
			0
		);
	}
	const QString filename_ = FileDialog::getInstance()->getOpenFileName(
		this,
		this->tr(
			"Select file"
		),
		defaultDir_
	);
	if(filename_.isEmpty() || !QFile::exists(
		filename_
	))
	{
		return;
	}
	QFile file_(
		filename_
	);
	if(!file_.open(
		QIODevice::ReadOnly
	))
	{
		MessageBox::warning(
			this,
			this->tr(
				"Error"
			),
			this->tr(
				"Unable to open file"
			).append(
				":\n"
			).append(
				file_.errorString()
			)
		);
		return;
	}
	QByteArray data_;
	if(!Tools::readAllFromDevice(
		&file_,
		data_
	))
	{
		MessageBox::warning(
			this,
			this->tr(
				"Error"
			),
			this->tr(
				"Unable to open file"
			).append(
				":\n"
			).append(
				file_.errorString()
			)
		);
		return;
	}
	this->entryAttachments->set(
		QFileInfo(
			filename_
		).fileName(),
		data_
	);
}

void EditEntryWidget::do_saveCurrentAttachment()
{
	const QModelIndex index_ = this->advancedUi->attachmentsView->
		currentIndex();
	if(!index_.isValid())
	{
		return;
	}
	const QString filename_ = this->attachmentsModel->keyByIndex(
		index_
	);
	QString defaultDirName_ = Config::getInstance()->get(
		"LastAttachmentDir"
	).toString();
	if(defaultDirName_.isEmpty() || !QDir(
		defaultDirName_
	).exists())
	{
		defaultDirName_ = QStandardPaths::writableLocation(
			QStandardPaths::DocumentsLocation
		);
	}
	const QDir dir_(
		defaultDirName_
	);
	if(const QString savePath_ = FileDialog::getInstance()->getSaveFileName(
			this,
			this->tr(
				"Save attachment"
			),
			dir_.filePath(
				filename_
			)
		);
		!savePath_.isEmpty())
	{
		const QByteArray attachmentData_ = this->entryAttachments->getValue(
			filename_
		);
		QFile file_(
			savePath_
		);
		if(!file_.open(
			QIODevice::WriteOnly
		))
		{
			MessageBox::warning(
				this,
				this->tr(
					"Error"
				),
				this->tr(
					"Unable to save the attachment:\n"
				).append(
					file_.errorString()
				)
			);
			return;
		}
		if(file_.write(
			attachmentData_
		) != attachmentData_.size())
		{
			MessageBox::warning(
				this,
				this->tr(
					"Error"
				),
				this->tr(
					"Unable to save the attachment:\n"
				).append(
					file_.errorString()
				)
			);
		}
	}
}

void EditEntryWidget::do_openAttachment(
	const QModelIndex &index
)
{
	if(!index.isValid())
	{
		return;
	}
	const QString filename_ = this->attachmentsModel->keyByIndex(
		index
	);
	const QByteArray attachmentData_ = this->entryAttachments->getValue(
		filename_
	);
	// tmp file will be removed once the database (or the application) has been closed
	const QString tmpFileTemplate_ = QDir::temp().absoluteFilePath(
		QString(
			"XXXXXX."
		).append(
			filename_
		)
	);
	const auto file_ = new QTemporaryFile(
		tmpFileTemplate_,
		this
	);
	if(!file_->open())
	{
		MessageBox::warning(
			this,
			this->tr(
				"Error"
			),
			this->tr(
				"Unable to save the attachment:\n"
			).append(
				file_->errorString()
			)
		);
		return;
	}
	if(file_->write(
		attachmentData_
	) != attachmentData_.size())
	{
		MessageBox::warning(
			this,
			this->tr(
				"Error"
			),
			this->tr(
				"Unable to save the attachment:\n"
			).append(
				file_->errorString()
			)
		);
		return;
	}
	if(!file_->flush())
	{
		MessageBox::warning(
			this,
			this->tr(
				"Error"
			),
			this->tr(
				"Unable to save the attachment:\n"
			).append(
				file_->errorString()
			)
		);
		return;
	}
	file_->close();
	QDesktopServices::openUrl(
		QUrl::fromLocalFile(
			file_->fileName()
		)
	);
}

void EditEntryWidget::do_openCurrentAttachment()
{
	const QModelIndex index_ = this->advancedUi->attachmentsView->
		currentIndex();
	this->do_openAttachment(
		index_
	);
}

void EditEntryWidget::do_removeCurrentAttachment() const
{
	if(this->history)
	{
		return;
	}
	const QModelIndex index_ = this->advancedUi->attachmentsView->
		currentIndex();
	if(!index_.isValid())
	{
		return;
	}
	const QString key_ = this->attachmentsModel->keyByIndex(
		index_
	);
	this->entryAttachments->remove(
		key_
	);
}

void EditEntryWidget::do_showHistoryEntry()
{
	if(const QModelIndex index_ = this->sortModel->mapToSource(
			this->historyUi->historyView->currentIndex()
		);
		index_.isValid())
	{
		this->do_emitHistoryEntryActivated(
			index_
		);
	}
}

void EditEntryWidget::do_restoreHistoryEntry() const
{
	if(const QModelIndex index_ = this->sortModel->mapToSource(
			this->historyUi->historyView->currentIndex()
		);
		index_.isValid())
	{
		this->setForms(
			this->historyModel->entryFromIndex(
				index_
			),
			true
		);
	}
}

void EditEntryWidget::do_deleteHistoryEntry() const
{
	if(const QModelIndex index_ = this->sortModel->mapToSource(
			this->historyUi->historyView->currentIndex()
		);
		index_.isValid())
	{
		this->historyModel->deleteIndex(
			index_
		);
		if(this->historyModel->rowCount() > 0)
		{
			this->historyUi->deleteAllButton->setEnabled(
				true
			);
		}
		else
		{
			this->historyUi->deleteAllButton->setEnabled(
				false
			);
		}
	}
}

void EditEntryWidget::do_deleteAllHistoryEntries() const
{
	this->historyModel->deleteAll();
	if(this->historyModel->rowCount() > 0)
	{
		this->historyUi->deleteAllButton->setEnabled(
			true
		);
	}
	else
	{
		this->historyUi->deleteAllButton->setEnabled(
			false
		);
	}
}

QMenu* EditEntryWidget::createPresetsMenu()
{
	const auto expirePresetsMenu_ = new QMenu(
		this
	);
	expirePresetsMenu_->addAction(
		this->tr(
			"Tomorrow"
		)
	)->setData(
		QVariant::fromValue(
			TimeDelta::fromDays(
				1
			)
		)
	);
	expirePresetsMenu_->addSeparator();
	expirePresetsMenu_->addAction(
		this->tr(
			"%n week(s)",
			nullptr,
			1
		)
	)->setData(
		QVariant::fromValue(
			TimeDelta::fromDays(
				7
			)
		)
	);
	expirePresetsMenu_->addAction(
		this->tr(
			"%n week(s)",
			nullptr,
			2
		)
	)->setData(
		QVariant::fromValue(
			TimeDelta::fromDays(
				14
			)
		)
	);
	expirePresetsMenu_->addAction(
		this->tr(
			"%n week(s)",
			nullptr,
			3
		)
	)->setData(
		QVariant::fromValue(
			TimeDelta::fromDays(
				21
			)
		)
	);
	expirePresetsMenu_->addSeparator();
	expirePresetsMenu_->addAction(
		this->tr(
			"%n month(s)",
			nullptr,
			1
		)
	)->setData(
		QVariant::fromValue(
			TimeDelta::fromMonths(
				1
			)
		)
	);
	expirePresetsMenu_->addAction(
		this->tr(
			"%n month(s)",
			nullptr,
			3
		)
	)->setData(
		QVariant::fromValue(
			TimeDelta::fromMonths(
				3
			)
		)
	);
	expirePresetsMenu_->addAction(
		this->tr(
			"%n month(s)",
			nullptr,
			6
		)
	)->setData(
		QVariant::fromValue(
			TimeDelta::fromMonths(
				6
			)
		)
	);
	expirePresetsMenu_->addSeparator();
	expirePresetsMenu_->addAction(
		this->tr(
			"1 year"
		)
	)->setData(
		QVariant::fromValue(
			TimeDelta::fromYears(
				1
			)
		)
	);
	return expirePresetsMenu_;
}
