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
#include "DatabaseWidget.h"
#include <QAction>
#include <QDesktopServices>
#include <QHeaderView>
#include <QKeyEvent>
#include <QProcess>
#include <QSplitter>
#include <QTimer>
#include "ui_SearchWidget.h"
#include "core/Config.h"
#include "core/EntrySearcher.h"
#include "core/FilePath.h"
#include "core/Global.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "core/Tools.h"
#include "gui/ChangeMasterKeyWidget.h"
#include "gui/Clipboard.h"
#include "gui/DatabaseOpenWidget.h"
#include "gui/DatabaseSettingsWidget.h"
#include "gui/MessageBox.h"
#include "gui/UnlockDatabaseWidget.h"
#include "gui/entry/EditEntryWidget.h"
#include "gui/entry/EntryView.h"
#include "gui/group/EditGroupWidget.h"
#include "gui/group/GroupView.h"

DatabaseWidget::DatabaseWidget(
	Database* db,
	QWidget* parent
)
	: QStackedWidget(
		parent
	),
	db(
		db
	),
	searchUi(
		new Ui::SearchWidget()
	),
	searchWidget(
		new QWidget()
	),
	newGroup(
		nullptr
	),
	newEntry(
		nullptr
	),
	newParent(
		nullptr
	)
{
	this->searchUi->setupUi(
		this->searchWidget
	);
	this->searchTimer = new QTimer(
		this
	);
	this->searchTimer->setSingleShot(
		true
	);
	this->mainWidget = new QWidget(
		this
	);
	QLayout* layout_ = new QHBoxLayout(
		this->mainWidget
	);
	this->splitter = new QSplitter(
		this->mainWidget
	);
	this->splitter->setChildrenCollapsible(
		false
	);
	const auto rightHandSideWidget_ = new QWidget(
		this->splitter
	);
	this->searchWidget->setParent(
		rightHandSideWidget_
	);
	this->groupView = new GroupView(
		this->db,
		this->splitter
	);
	this->groupView->setObjectName(
		"groupView"
	);
	this->groupView->setContextMenuPolicy(
		Qt::CustomContextMenu
	);
	this->connect(
		this->groupView,
		&GroupView::customContextMenuRequested,
		this,
		&DatabaseWidget::do_emitGroupContextMenuRequested
	);
	this->entryView = new EntryView(
		rightHandSideWidget_
	);
	this->entryView->setObjectName(
		"entryView"
	);
	this->entryView->setContextMenuPolicy(
		Qt::CustomContextMenu
	);
	this->entryView->do_setGroup(
		this->db->getRootGroup()
	);
	this->connect(
		this->entryView,
		&EntryView::customContextMenuRequested,
		this,
		&DatabaseWidget::do_emitEntryContextMenuRequested
	);
	const auto closeAction_ = new QAction(
		this->searchWidget
	);
	const QIcon closeIcon_ = FilePath::getInstance()->getIcon(
		"actions",
		"dialog-close"
	);
	closeAction_->setIcon(
		closeIcon_
	);
	this->searchUi->closeSearchButton->setDefaultAction(
		closeAction_
	);
	this->searchUi->closeSearchButton->setShortcut(
		Qt::Key_Escape
	);
	this->searchWidget->hide();
	this->searchUi->caseSensitiveCheckBox->setVisible(
		false
	);
	this->searchUi->searchEdit->installEventFilter(
		this
	);
	const auto vLayout_ = new QVBoxLayout(
		rightHandSideWidget_
	);
	vLayout_->setContentsMargins(
		0,
		0,
		0,
		0
	);
	vLayout_->addWidget(
		this->searchWidget
	);
	vLayout_->addWidget(
		this->entryView
	);
	rightHandSideWidget_->setLayout(
		vLayout_
	);
	this->setTabOrder(
		this->searchUi->searchRootRadioButton,
		this->entryView
	);
	this->setTabOrder(
		this->entryView,
		this->groupView
	);
	this->setTabOrder(
		this->groupView,
		this->searchWidget
	);
	this->splitter->addWidget(
		this->groupView
	);
	this->splitter->addWidget(
		rightHandSideWidget_
	);
	this->splitter->setStretchFactor(
		0,
		30
	);
	this->splitter->setStretchFactor(
		1,
		70
	);
	layout_->addWidget(
		this->splitter
	);
	this->mainWidget->setLayout(
		layout_
	);
	this->editEntryWidget = new EditEntryWidget();
	this->editEntryWidget->setObjectName(
		"editEntryWidget"
	);
	this->historyEditEntryWidget = new EditEntryWidget();
	this->editGroupWidget = new EditGroupWidget();
	this->editGroupWidget->setObjectName(
		"editGroupWidget"
	);
	this->changeMasterKeyWidget = new ChangeMasterKeyWidget();
	this->changeMasterKeyWidget->headlineLabel()->setText(
		this->tr(
			"Change master key"
		)
	);
	QFont headlineLabelFont_ = this->changeMasterKeyWidget->headlineLabel()->
		font();
	headlineLabelFont_.setBold(
		true
	);
	headlineLabelFont_.setPointSize(
		headlineLabelFont_.pointSize() + 2
	);
	this->changeMasterKeyWidget->headlineLabel()->setFont(
		headlineLabelFont_
	);
	this->databaseSettingsWidget = new DatabaseSettingsWidget();
	this->databaseSettingsWidget->setObjectName(
		"databaseSettingsWidget"
	);
	this->databaseOpenWidget = new DatabaseOpenWidget();
	this->databaseOpenWidget->setObjectName(
		"databaseOpenWidget"
	);
	this->unlockDatabaseWidget = new UnlockDatabaseWidget();
	this->unlockDatabaseWidget->setObjectName(
		"unlockDatabaseWidget"
	);
	this->addDbWidget(
		this->mainWidget
	);
	this->addDbWidget(
		this->editEntryWidget
	);
	this->addDbWidget(
		this->editGroupWidget
	);
	this->addDbWidget(
		this->changeMasterKeyWidget
	);
	this->addDbWidget(
		this->databaseSettingsWidget
	);
	this->addDbWidget(
		this->historyEditEntryWidget
	);
	this->addDbWidget(
		this->databaseOpenWidget
	);
	this->addDbWidget(
		this->unlockDatabaseWidget
	);
	this->connect(
		this->splitter,
		&QSplitter::splitterMoved,
		this,
		&DatabaseWidget::sig_splitterSizesChanged
	);
	this->connect(
		this->entryView->header(),
		&QHeaderView::sectionResized,
		this,
		&DatabaseWidget::sig_entryColumnSizesChanged
	);
	this->connect(
		this->groupView,
		&GroupView::sig_groupChanged,
		this,
		&DatabaseWidget::do_clearLastGroup
	);
	this->connect(
		this->groupView,
		&GroupView::sig_groupChanged,
		this,
		&DatabaseWidget::sig_groupChanged
	);
	this->connect(
		this->groupView,
		&GroupView::sig_groupChanged,
		this->entryView,
		&EntryView::do_setGroup
	);
	this->connect(
		this->entryView,
		&EntryView::sig_entryActivated,
		this,
		&DatabaseWidget::do_entryActivationSignalReceived
	);
	this->connect(
		this->entryView,
		&EntryView::sig_entrySelectionChanged,
		this,
		&DatabaseWidget::sig_entrySelectionChanged
	);
	this->connect(
		this->editEntryWidget,
		&EditEntryWidget::sig_editFinished,
		this,
		&DatabaseWidget::do_switchToView
	);
	this->connect(
		this->editEntryWidget,
		&EditEntryWidget::sig_historyEntryActivated,
		this,
		&DatabaseWidget::do_switchToHistoryView
	);
	this->connect(
		this->historyEditEntryWidget,
		&EditEntryWidget::sig_editFinished,
		this,
		&DatabaseWidget::do_switchBackToEntryEdit
	);
	this->connect(
		this->editGroupWidget,
		&EditGroupWidget::sig_editFinished,
		this,
		&DatabaseWidget::do_switchToView
	);
	this->connect(
		this->changeMasterKeyWidget,
		&ChangeMasterKeyWidget::sig_editFinished,
		this,
		&DatabaseWidget::do_updateMasterKey
	);
	this->connect(
		this->databaseSettingsWidget,
		&DatabaseSettingsWidget::sig_editFinished,
		this,
		&DatabaseWidget::do_switchToView
	);
	this->connect(
		this->databaseOpenWidget,
		&DatabaseOpenWidget::sig_editFinished,
		this,
		&DatabaseWidget::do_openDatabase
	);
	this->connect(
		this->unlockDatabaseWidget,
		&UnlockDatabaseWidget::sig_editFinished,
		this,
		&DatabaseWidget::do_unlockDatabase
	);
	this->connect(
		this,
		&DatabaseWidget::currentChanged,
		this,
		&DatabaseWidget::do_emitCurrentModeChanged
	);
	this->connect(
		this->searchUi->searchEdit,
		&QLineEdit::textChanged,
		this,
		&DatabaseWidget::do_startSearchTimer
	);
	this->connect(
		this->searchUi->caseSensitiveCheckBox,
		&QCheckBox::toggled,
		this,
		&DatabaseWidget::do_startSearch
	);
	this->connect(
		this->searchUi->searchCurrentRadioButton,
		&QRadioButton::toggled,
		this,
		&DatabaseWidget::do_startSearch
	);
	this->connect(
		this->searchUi->searchRootRadioButton,
		&QRadioButton::toggled,
		this,
		&DatabaseWidget::do_startSearch
	);
	this->connect(
		this->searchUi->searchEdit,
		&LineEdit::returnPressed,
		this->entryView,
		&EntryView::do_setFocus
	);
	this->connect(
		this->searchTimer,
		&QTimer::timeout,
		this,
		&DatabaseWidget::do_search
	);
	this->connect(
		closeAction_,
		&QAction::triggered,
		this,
		&DatabaseWidget::do_closeSearch
	);
	this->setCurrentDbWidget(
		this->mainWidget
	);
}

DatabaseWidget::~DatabaseWidget()
{
	/*if(this->db != nullptr)
	{
		delete(this->db);
		this->db = nullptr;
	}*/
}

DatabaseWidget::Mode DatabaseWidget::getCurrentMode() const
{
	if(this->currentWidget() == nullptr)
	{
		return this->None;
	}
	if(this->currentWidget() == this->mainWidget)
	{
		return this->ViewMode;
	}
	if(this->currentWidget() == this->unlockDatabaseWidget)
	{
		return this->LockedMode;
	}
	return this->EditMode;
}

bool DatabaseWidget::isInEditMode() const
{
	return this->getCurrentMode() == this->EditMode;
}

bool DatabaseWidget::isEditWidgetModified() const
{
	if(this->currentWidget() == this->editEntryWidget)
	{
		return this->editEntryWidget->hasBeenModified();
	}
	// other edit widget don't have a hasBeenModified() method yet
	// assume that they already have been modified
	return true;
}

QList<int> DatabaseWidget::getSplitterSizes() const
{
	return this->splitter->sizes();
}

void DatabaseWidget::setSplitterSizes(
	const QList<int> &sizes
) const
{
	this->splitter->setSizes(
		sizes
	);
}

QList<int> DatabaseWidget::getEntryHeaderViewSizes() const
{
	QList<int> sizes_;
	for(auto i_ = 0; i_ < this->entryView->header()->count(); i_++)
	{
		sizes_.append(
			this->entryView->header()->sectionSize(
				i_
			)
		);
	}
	return sizes_;
}

void DatabaseWidget::setEntryViewHeaderSizes(
	const QList<int> &sizes
) const
{
	if(sizes.size() != this->entryView->header()->count())
	{
		return;
	}
	for(auto i_ = 0; i_ < sizes.size(); i_++)
	{
		this->entryView->header()->resizeSection(
			i_,
			sizes[i_]
		);
	}
}

void DatabaseWidget::clearAllWidgets() const
{
	this->editEntryWidget->clear();
	this->historyEditEntryWidget->clear();
	this->editGroupWidget->clear();
}

void DatabaseWidget::do_emitCurrentModeChanged()
{
	this->sig_currentModeChanged(
		this->getCurrentMode()
	);
}

Database* DatabaseWidget::getDatabase() const
{
	return this->db;
}

void DatabaseWidget::do_createEntry()
{
	if(!this->groupView->getCurrentGroup())
	{
		return;
	}
	this->newEntry = new Entry();
	this->newEntry->setUUID(
		UUID::random()
	);
	this->newEntry->setUsername(
		this->db->getMetadata()->getDefaultUserName()
	);
	this->newParent = this->groupView->getCurrentGroup();
	this->setIconFromParent();
	this->do_switchToEntryEdit(
		this->newEntry,
		true
	);
}

void DatabaseWidget::setIconFromParent() const
{
	if(!Config::getInstance()->get(
		"UseGroupIconOnEntryCreation"
	).toBool())
	{
		return;
	}
	if(this->newParent->getIconNumber() == Group::DefaultIconNumber && this->
		newParent->getIconUUID().isNull())
	{
		return;
	}
	if(this->newParent->getIconUUID().isNull())
	{
		this->newEntry->setIcon(
			this->newParent->getIconNumber()
		);
	}
	else
	{
		this->newEntry->setIcon(
			this->newParent->getIconUUID()
		);
	}
}

void DatabaseWidget::replaceDatabase(
	Database* db
)
{
	const Database* oldDb_ = this->db;
	this->db = db;
	groupView->changeDatabase(
		this->db
	);
	this->sig_databaseChanged(
		this->db
	);
	delete oldDb_;
}

void DatabaseWidget::do_cloneEntry() const
{
	Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return;
	}
	Entry* entry = currentEntry_->clone(
		Entry::CloneNewUuid | Entry::CloneResetTimeInfo
	);
	entry->setGroup(
		currentEntry_->getGroup()
	);
	this->entryView->setFocus();
	this->entryView->setCurrentEntry(
		entry
	);
}

void DatabaseWidget::do_deleteEntries()
{
	const QModelIndexList selected_ = this->entryView->selectionModel()->
		selectedRows();
	if(selected_.isEmpty())
	{
		return;
	}
	// get all entry pointers as the indexes change when removing multiple entries
	QList<Entry*> selectedEntries_;
	for(const QModelIndex &index_: selected_)
	{
		selectedEntries_.append(
			this->entryView->entryFromIndex(
				index_
			)
		);
	}
	if(const bool inRecylceBin_ = Tools::hasChild(
			this->db->getMetadata()->getRecycleBin(),
			selectedEntries_.first()
		);
		inRecylceBin_ || !this->db->getMetadata()->recycleBinEnabled())
	{
		QMessageBox::StandardButton result_;
		if(selected_.size() == 1)
		{
			result_ = MessageBox::question(
				this,
				this->tr(
					"Delete entry?"
				),
				this->tr(
					"Do you really want to delete the entry \"%1\" for good?"
				).arg(
					selectedEntries_.first()->getTitle()
				),
				QMessageBox::Yes | QMessageBox::No
			);
		}
		else
		{
			result_ = MessageBox::question(
				this,
				this->tr(
					"Delete entries?"
				),
				this->tr(
					"Do you really want to delete %1 entries for good?"
				).arg(
					selected_.size()
				),
				QMessageBox::Yes | QMessageBox::No
			);
		}
		if(result_ == QMessageBox::Yes)
		{
			for(const Entry* entry_: asConst(
					selectedEntries_
				))
			{
				delete entry_;
			}
		}
	}
	else
	{
		QMessageBox::StandardButton result_;
		if(selected_.size() == 1)
		{
			result_ = MessageBox::question(
				this,
				this->tr(
					"Move entry to recycle bin?"
				),
				this->tr(
					"Do you really want to move entry \"%1\" to the recycle bin?"
				).arg(
					selectedEntries_.first()->getTitle()
				),
				QMessageBox::Yes | QMessageBox::No
			);
		}
		else
		{
			result_ = MessageBox::question(
				this,
				this->tr(
					"Move entries to recycle bin?"
				),
				this->tr(
					"Do you really want to move %n entry(s) to the recycle bin?",
					nullptr,
					static_cast<int>(selected_.size())
				),
				QMessageBox::Yes | QMessageBox::No
			);
		}
		if(result_ == QMessageBox::No)
		{
			return;
		}
		for(Entry* entry_: asConst(
				selectedEntries_
			))
		{
			this->db->recycleEntry(
				entry_
			);
		}
	}
}

void DatabaseWidget::do_copyTitle() const
{
	const Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return;
	}
	this->setClipboardTextAndMinimize(
		currentEntry_->getTitle()
	);
}

void DatabaseWidget::do_copyUsername() const
{
	const Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return;
	}
	this->setClipboardTextAndMinimize(
		currentEntry_->getUsername()
	);
}

void DatabaseWidget::do_copyPassword() const
{
	const Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return;
	}
	this->setClipboardTextAndMinimize(
		currentEntry_->getPassword()
	);
}

void DatabaseWidget::do_copyURL() const
{
	const Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return;
	}
	this->setClipboardTextAndMinimize(
		currentEntry_->getURL()
	);
}

void DatabaseWidget::do_copyNotes() const
{
	const Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return;
	}
	this->setClipboardTextAndMinimize(
		currentEntry_->getNotes()
	);
}

void DatabaseWidget::do_copyAttribute(
	const QAction* action
) const
{
	Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return;
	}
	this->setClipboardTextAndMinimize(
		currentEntry_->getAttributes()->getValue(
			action->text()
		)
	);
}

void DatabaseWidget::setClipboardTextAndMinimize(
	const QString &text
) const
{
	Clipboard::getInstance()->setText(
		text
	);
	if(Config::getInstance()->get(
		"MinimizeOnCopy"
	).toBool())
	{
		this->window()->showMinimized();
	}
}

void DatabaseWidget::do_openUrl() const
{
	const Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return;
	}
	this->do_openUrlForEntry(
		currentEntry_
	);
}

void DatabaseWidget::do_openUrlForEntry(
	const Entry* entry
)
{
	const QString urlString_ = entry->resolvePlaceholders(
		entry->getURL()
	);
	if(urlString_.isEmpty())
	{
		return;
	}
	if(urlString_.startsWith(
		"cmd://"
	))
	{
		if(urlString_.length() > 6)
		{
			QProcess::startDetached(
				urlString_.mid(
					6
				)
			);
		}
	}
	else
	{
		const QUrl url = QUrl::fromUserInput(
			urlString_
		);
		QDesktopServices::openUrl(
			url
		);
	}
}

void DatabaseWidget::do_createGroup()
{
	if(!this->groupView->getCurrentGroup())
	{
		return;
	}
	this->newGroup = new Group();
	this->newGroup->setUuid(
		UUID::random()
	);
	this->newParent = this->groupView->getCurrentGroup();
	this->do_switchToGroupEdit(
		this->newGroup,
		true
	);
}

void DatabaseWidget::do_deleteGroup()
{
	Group* currentGroup_ = this->groupView->getCurrentGroup();
	if(!currentGroup_ || !this->canDeleteCurrentGroup())
	{
		return;
	}
	const bool inRecylceBin_ = Tools::hasChild(
		this->db->getMetadata()->getRecycleBin(),
		currentGroup_
	);
	const bool isRecycleBin_ = (currentGroup_ == this->db->getMetadata()->
		getRecycleBin());
	if(const bool isRecycleBinSubgroup_ = Tools::hasChild(
			currentGroup_,
			this->db->getMetadata()->getRecycleBin()
		);
		inRecylceBin_ || isRecycleBin_ || isRecycleBinSubgroup_ || !this->db->
		getMetadata()->recycleBinEnabled())
	{
		if(const QMessageBox::StandardButton result_ = MessageBox::question(
				this,
				this->tr(
					"Delete group?"
				),
				this->tr(
					"Do you really want to delete the group \"%1\" for good?"
				).arg(
					currentGroup_->getName()
				),
				QMessageBox::Yes | QMessageBox::No
			);
			result_ == QMessageBox::Yes)
		{
			delete currentGroup_;
		}
	}
	else
	{
		this->db->recycleGroup(
			currentGroup_
		);
	}
}

int DatabaseWidget::addDbWidget(
	QWidget* w
)
{
	w->setSizePolicy(
		QSizePolicy::Ignored,
		QSizePolicy::Ignored
	);
	const int index_ = addWidget(
		w
	);
	this->adjustSize();
	return index_;
}

void DatabaseWidget::setCurrentDbIndex(
	const int index
)
{
	// use setCurrentWidget() instead
	// index is not reliable
	Q_UNUSED(
		index
	);
}

void DatabaseWidget::setCurrentDbWidget(
	QWidget* widget
)
{
	if(this->currentWidget())
	{
		this->currentWidget()->setSizePolicy(
			QSizePolicy::Ignored,
			QSizePolicy::Ignored
		);
	}
	this->setCurrentWidget(
		widget
	);
	if(this->currentWidget())
	{
		this->currentWidget()->setSizePolicy(
			QSizePolicy::Preferred,
			QSizePolicy::Preferred
		);
	}
	this->adjustSize();
}

void DatabaseWidget::do_switchToView(
	const bool accepted
)
{
	if(this->newGroup)
	{
		if(accepted)
		{
			this->newGroup->setParent(
				this->newParent
			);
			this->groupView->setCurrentGroup(
				this->newGroup
			);
			this->groupView->expandGroup(
				this->newParent
			);
		}
		else
		{
			delete this->newGroup;
		}
		this->newGroup = nullptr;
		this->newParent = nullptr;
	}
	else if(this->newEntry)
	{
		if(accepted)
		{
			this->newEntry->setGroup(
				this->newParent
			);
			this->entryView->setFocus();
			this->entryView->setCurrentEntry(
				this->newEntry
			);
		}
		else
		{
			delete this->newEntry;
		}
		this->newEntry = nullptr;
		this->newParent = nullptr;
	}
	this->setCurrentDbWidget(
		this->mainWidget
	);
}

void DatabaseWidget::do_switchToHistoryView(
	Entry* entry
)
{
	this->historyEditEntryWidget->loadEntry(
		entry,
		false,
		true,
		this->editEntryWidget->entryTitle(),
		this->db
	);
	this->setCurrentWidget(
		this->historyEditEntryWidget
	);
}

void DatabaseWidget::do_switchBackToEntryEdit()
{
	this->setCurrentWidget(
		this->editEntryWidget
	);
}

void DatabaseWidget::do_switchToEntryEdit(
	Entry* entry
)
{
	this->do_switchToEntryEdit(
		entry,
		false
	);
}

void DatabaseWidget::do_switchToEntryEdit(
	Entry* entry,
	const bool create
)
{
	const Group* group_ = this->groupView->getCurrentGroup();
	if(!group_)
	{
		group_ = this->lastGroup;
	}
	this->editEntryWidget->loadEntry(
		entry,
		create,
		false,
		group_->getName(),
		this->db
	);
	this->setCurrentWidget(
		this->editEntryWidget
	);
}

void DatabaseWidget::do_switchToGroupEdit(
	Group* entry,
	const bool create
)
{
	this->editGroupWidget->loadGroup(
		entry,
		create,
		this->db
	);
	this->setCurrentWidget(
		this->editGroupWidget
	);
}

void DatabaseWidget::do_updateMasterKey(
	const bool accepted
)
{
	if(accepted)
	{
		QApplication::setOverrideCursor(
			QCursor(
				Qt::WaitCursor
			)
		);
		const bool result_ = this->db->setKey(
			this->changeMasterKeyWidget->newMasterKey()
		);
		QApplication::restoreOverrideCursor();
		if(!result_)
		{
			MessageBox::critical(
				this,
				tr(
					"Error"
				),
				tr(
					"Unable to calculate master key"
				)
			);
			return;
		}
	}
	else if(!this->db->hasKey())
	{
		this->sig_closeRequest();
		return;
	}
	this->setCurrentDbWidget(
		this->mainWidget
	);
}

void DatabaseWidget::do_openDatabase(
	const bool accepted
)
{
	if(accepted)
	{
		this->replaceDatabase(
			static_cast<DatabaseOpenWidget*>(this->sender())->database()
		);
		this->setCurrentDbWidget(
			this->mainWidget
		);
		// We won't need those anymore and KeePass1OpenWidget closes
		// the file in its dtor.
		delete this->databaseOpenWidget;
		this->databaseOpenWidget = nullptr;
	}
	else
	{
		if(this->databaseOpenWidget->database() != nullptr)
		{
			delete this->databaseOpenWidget->database();
		}
		this->sig_closeRequest();
	}
}

void DatabaseWidget::do_unlockDatabase(
	const bool accepted
)
{
	if(!accepted)
	{
		this->sig_closeRequest();
		return;
	}
	replaceDatabase(
		static_cast<DatabaseOpenWidget*>(this->sender())->database()
	);
	const QList<Group*> groups_ = this->db->getRootGroup()->getGroupsRecursive(
		true
	);
	for(Group* group_: groups_)
	{
		if(group_->getUUID() == this->groupBeforeLock)
		{
			this->groupView->setCurrentGroup(
				group_
			);
			break;
		}
	}
	this->groupBeforeLock = UUID();
	this->setCurrentDbWidget(
		this->mainWidget
	);
	this->unlockDatabaseWidget->clearForms();
	this->sig_unlockedDatabase();
}

void DatabaseWidget::do_entryActivationSignalReceived(
	Entry* entry,
	const EntryModel::ModelColumn column
)
{
	if(column == EntryModel::Url && !entry->getURL().isEmpty())
	{
		this->do_openUrlForEntry(
			entry
		);
	}
	else
	{
		this->do_switchToEntryEdit(
			entry
		);
	}
}

void DatabaseWidget::do_switchToEntryEdit()
{
	Entry* entry_ = this->entryView->getCurrentEntry();
	if(!entry_)
	{
		return;
	}
	this->do_switchToEntryEdit(
		entry_,
		false
	);
}

void DatabaseWidget::do_switchToGroupEdit()
{
	Group* group_ = this->groupView->getCurrentGroup();
	if(!group_)
	{
		return;
	}
	this->do_switchToGroupEdit(
		group_,
		false
	);
}

void DatabaseWidget::do_switchToMasterKeyChange()
{
	this->changeMasterKeyWidget->clearForms();
	this->setCurrentDbWidget(
		this->changeMasterKeyWidget
	);
}

void DatabaseWidget::do_switchToDatabaseSettings()
{
	this->databaseSettingsWidget->load(
		this->db
	);
	this->setCurrentDbWidget(
		this->databaseSettingsWidget
	);
}

void DatabaseWidget::do_switchToOpenDatabase(
	const QString &fileName
)
{
	this->updateFilename(
		fileName
	);
	this->databaseOpenWidget->load(
		fileName
	);
	this->setCurrentDbWidget(
		this->databaseOpenWidget
	);
}

void DatabaseWidget::do_switchToOpenDatabase(
	const QString &fileName,
	const QString &password,
	const QString &keyFile
)
{
	this->updateFilename(
		fileName
	);
	this->do_switchToOpenDatabase(
		fileName
	);
	this->databaseOpenWidget->enterKey(
		password,
		keyFile
	);
}

void DatabaseWidget::do_openSearch()
{
	if(this->isInSearchMode())
	{
		this->searchUi->searchEdit->selectAll();
		if(!this->searchUi->searchEdit->hasFocus())
		{
			this->searchUi->searchEdit->setFocus();
			// make sure the search action is checked again
			this->do_emitCurrentModeChanged();
		}
	}
	else
	{
		this->do_showSearch();
	}
}

void DatabaseWidget::do_closeSearch()
{
	this->sig_listModeAboutToActivate();
	this->groupView->setCurrentGroup(
		this->lastGroup
	);
	this->searchTimer->stop();
	this->sig_listModeActivated();
}

void DatabaseWidget::do_showSearch()
{
	this->sig_searchModeAboutToActivate();
	this->searchUi->searchEdit->blockSignals(
		true
	);
	this->searchUi->searchEdit->clear();
	this->searchUi->searchEdit->blockSignals(
		false
	);
	this->searchUi->searchCurrentRadioButton->blockSignals(
		true
	);
	this->searchUi->searchRootRadioButton->blockSignals(
		true
	);
	this->searchUi->searchRootRadioButton->setChecked(
		true
	);
	this->searchUi->searchCurrentRadioButton->blockSignals(
		false
	);
	this->searchUi->searchRootRadioButton->blockSignals(
		false
	);
	this->lastGroup = this->groupView->getCurrentGroup();
	if(this->lastGroup == this->db->getRootGroup())
	{
		this->searchUi->optionsWidget->hide();
		this->searchUi->searchCurrentRadioButton->hide();
		this->searchUi->searchRootRadioButton->hide();
	}
	else
	{
		this->searchUi->optionsWidget->show();
		this->searchUi->searchCurrentRadioButton->show();
		this->searchUi->searchRootRadioButton->show();
		this->searchUi->searchCurrentRadioButton->setText(
			this->tr(
				"Current group"
			).append(
				" ("
			).append(
				lastGroup->getName()
			).append(
				")"
			)
		);
	}
	this->groupView->setCurrentIndex(
		QModelIndex()
	);
	this->searchWidget->show();
	this->do_search();
	this->searchUi->searchEdit->setFocus();
	this->sig_searchModeActivated();
}

void DatabaseWidget::do_search() const
{
	Group* searchGroup_;
	if(this->searchUi->searchCurrentRadioButton->isChecked())
	{
		searchGroup_ = this->lastGroup;
	}
	else if(this->searchUi->searchRootRadioButton->isChecked())
	{
		searchGroup_ = this->db->getRootGroup();
	}
	else
	{
		return;
	}
	Qt::CaseSensitivity sensitivity_;
	if(this->searchUi->caseSensitiveCheckBox->isChecked())
	{
		sensitivity_ = Qt::CaseSensitive;
	}
	else
	{
		sensitivity_ = Qt::CaseInsensitive;
	}
	const QList<Entry*> searchResult_ = EntrySearcher().search(
		this->searchUi->searchEdit->text(),
		searchGroup_,
		sensitivity_
	);
	this->entryView->setEntryList(
		searchResult_
	);
}

void DatabaseWidget::do_startSearchTimer() const
{
	if(!this->searchTimer->isActive())
	{
		this->searchTimer->stop();
	}
	this->searchTimer->start(
		100
	);
}

void DatabaseWidget::do_startSearch() const
{
	if(!this->searchTimer->isActive())
	{
		this->searchTimer->stop();
	}
	this->do_search();
}

void DatabaseWidget::do_emitGroupContextMenuRequested(
	const QPoint &pos
)
{
	this->sig_groupContextMenuRequested(
		this->groupView->viewport()->mapToGlobal(
			pos
		)
	);
}

void DatabaseWidget::do_emitEntryContextMenuRequested(
	const QPoint &pos
)
{
	this->sig_entryContextMenuRequested(
		this->entryView->viewport()->mapToGlobal(
			pos
		)
	);
}

bool DatabaseWidget::dbHasKey() const
{
	return this->db->hasKey();
}

bool DatabaseWidget::canDeleteCurrentGroup() const
{
	const bool isRootGroup = this->db->getRootGroup() == this->groupView->
		getCurrentGroup();
	return !isRootGroup;
}

bool DatabaseWidget::isInSearchMode() const
{
	return this->entryView->isInEntryListMode();
}

void DatabaseWidget::do_clearLastGroup(
	const Group* group
)
{
	if(group)
	{
		this->lastGroup = nullptr;
		this->searchWidget->hide();
	}
}

void DatabaseWidget::lock()
{
	if(this->isInSearchMode())
	{
		this->do_closeSearch();
	}
	if(this->groupView->getCurrentGroup())
	{
		this->groupBeforeLock = this->groupView->getCurrentGroup()->getUUID();
	}
	else
	{
		this->groupBeforeLock = this->db->getRootGroup()->getUUID();
	}
	this->clearAllWidgets();
	this->unlockDatabaseWidget->load(
		this->filename
	);
	this->setCurrentDbWidget(
		this->unlockDatabaseWidget
	);
	const auto newDb_ = new Database();
	newDb_->getMetadata()->setName(
		this->db->getMetadata()->getName()
	);
	this->replaceDatabase(
		newDb_
	);
}

void DatabaseWidget::updateFilename(
	const QString &filename
)
{
	this->filename = filename;
}

int DatabaseWidget::getNumberOfSelectedEntries() const
{
	return static_cast<int>(this->entryView->numberOfSelectedEntries());
}

QStringList DatabaseWidget::getCustomEntryAttributes() const
{
	Entry* entry_ = this->entryView->getCurrentEntry();
	if(!entry_)
	{
		return QStringList();
	}
	return entry_->getAttributes()->getCustomKeys();
}

bool DatabaseWidget::isGroupSelected() const
{
	return this->groupView->getCurrentGroup() != nullptr;
}

bool DatabaseWidget::currentEntryHasTitle() const
{
	const Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return false;
	}
	return !currentEntry_->getTitle().isEmpty();
}

bool DatabaseWidget::currentEntryHasUsername() const
{
	const Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return false;
	}
	return !currentEntry_->getUsername().isEmpty();
}

bool DatabaseWidget::currentEntryHasPassword() const
{
	const Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return false;
	}
	return !currentEntry_->getPassword().isEmpty();
}

bool DatabaseWidget::currentEntryHasUrl() const
{
	const Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return false;
	}
	return !currentEntry_->getURL().isEmpty();
}

bool DatabaseWidget::currentEntryHasNotes() const
{
	const Entry* currentEntry_ = this->entryView->getCurrentEntry();
	if(!currentEntry_)
	{
		return false;
	}
	return !currentEntry_->getNotes().isEmpty();
}

bool DatabaseWidget::eventFilter(
	QObject* object,
	QEvent* event
)
{
	if(object == this->searchUi->searchEdit)
	{
		if(event->type() == QEvent::KeyPress)
		{
			if(const auto keyEvent_ = static_cast<QKeyEvent*>(event);
				keyEvent_->matches(
					QKeySequence::Copy
				))
			{
				// If Control+C is pressed in the search edit when no
				// text is selected, copy the password of the current
				// entry.
				if(const Entry* currentEntry_ = this->entryView->
						getCurrentEntry();
					currentEntry_ && !this->searchUi->searchEdit->
					hasSelectedText())
				{
					this->setClipboardTextAndMinimize(
						currentEntry_->getPassword()
					);
					return true;
				}
			}
			else if(keyEvent_->matches(
				QKeySequence::MoveToNextLine
			))
			{
				// If Down is pressed at EOL in the search edit, move
				// the focus to the entry view.
				if(!this->searchUi->searchEdit->hasSelectedText() && this->
					searchUi->searchEdit->cursorPosition() == this->searchUi->
					searchEdit->text().size())
				{
					this->entryView->setFocus();
					return true;
				}
			}
		}
	}
	return false;
}
