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
#ifndef KEEPASSX_EDITENTRYWIDGET_H
#define KEEPASSX_EDITENTRYWIDGET_H
#include <QModelIndex>
#include <QScopedPointer>
#include "gui/EditWidget.h"
class Database;
class EditWidgetIcons;
class EditWidgetProperties;
class Entry;
class EntryAttachments;
class EntryAttachmentsModel;
class EntryAttributes;
class EntryAttributesModel;
class EntryHistoryModel;
class QButtonGroup;
class QMenu;
class QSortFilterProxyModel;
class QStackedLayout;

namespace Ui
{
	class EditEntryWidgetAdvanced;
	class EditEntryWidgetMain;
	class EditEntryWidgetHistory;
	class EditWidget;
}

class EditEntryWidget final:public EditWidget
{
	Q_OBJECT public:
	explicit EditEntryWidget(
		QWidget* parent = nullptr
	);
	virtual ~EditEntryWidget() override;
	void loadEntry(
		Entry* entry,
		bool create,
		bool history,
		const QString &parentName,
		Database* database
	);
	void createPresetsMenu(
		QMenu* expirePresetsMenu
	);
	QString entryTitle() const;
	void clear();
	bool hasBeenModified() const;
Q_SIGNALS:
	void sig_editFinished(
		bool accepted
	);
	void sig_historyEntryActivated(
		Entry* entry
	);
private Q_SLOTS:
	void do_saveEntry();
	void do_cancel();
	void do_togglePasswordGeneratorButton(
		bool checked
	) const;
	void do_setGeneratedPassword(
		const QString &password
	) const;
	void do_insertAttribute() const;
	void do_editCurrentAttribute() const;
	void do_removeCurrentAttribute() const;
	void do_updateCurrentAttribute();
	void do_insertAttachment();
	void do_saveCurrentAttachment();
	void do_openAttachment(
		const QModelIndex &index
	);
	void do_openCurrentAttachment();
	void do_removeCurrentAttachment() const;
	void do_showHistoryEntry();
	void do_restoreHistoryEntry() const;
	void do_deleteHistoryEntry() const;
	void do_deleteAllHistoryEntries() const;
	void do_emitHistoryEntryActivated(
		const QModelIndex &index
	);
	void do_histEntryActivated(
		const QModelIndex &index
	);
	void do_updateHistoryButtons(
		const QModelIndex &current,
		const QModelIndex &previous
	) const;
	void do_useExpiryPreset(
		const QAction* action
	) const;
	void do_updateAttachmentButtonsEnabled(
		const QModelIndex &current
	) const;
private:
	void setupMain();
	void setupAdvanced() const;
	void setupIcon() const;
	void setupProperties() const;
	void setupHistory() const;
	bool passwordsEqual() const;
	void setForms(
		const Entry* entry,
		bool restore = false
	) const;
	QMenu* createPresetsMenu();
	void updateEntryData(
		Entry* entry
	) const;
	Entry* entry;
	Database* database;
	bool create;
	bool history;
	const QScopedPointer<Ui::EditEntryWidgetMain> mainUi;
	const QScopedPointer<Ui::EditEntryWidgetAdvanced> advancedUi;
	const QScopedPointer<Ui::EditEntryWidgetHistory> historyUi;
	QWidget* const mainWidget;
	QWidget* const advancedWidget;
	EditWidgetIcons* const iconsWidget;
	EditWidgetProperties* const editWidgetProperties;
	QWidget* const historyWidget;
	EntryAttachments* const entryAttachments;
	EntryAttachmentsModel* const attachmentsModel;
	EntryAttributes* const entryAttributes;
	EntryAttributesModel* const attributesModel;
	EntryHistoryModel* const historyModel;
	QSortFilterProxyModel* const sortModel;
	QPersistentModelIndex currentAttribute;
	Q_DISABLE_COPY(
		EditEntryWidget
	)
};
#endif // KEEPASSX_EDITENTRYWIDGET_H
