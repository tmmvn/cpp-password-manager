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
#include "EntryView.h"
#include <QHeaderView>
#include <QKeyEvent>
#include "gui/SortFilterHideProxyModel.h"

EntryView::EntryView(
	QWidget* parent
)
	: QTreeView(
		parent
	),
	model(
		new EntryModel(
			this
		)
	),
	sortModel(
		new SortFilterHideProxyModel(
			this
		)
	),
	inEntryListMode(
		false
	)
{
	this->sortModel->setSourceModel(
		model
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
	QTreeView::setModel(
		this->sortModel
	);
	this->setUniformRowHeights(
		true
	);
	this->setRootIsDecorated(
		false
	);
	this->setAlternatingRowColors(
		true
	);
	this->setDragEnabled(
		true
	);
	this->setSortingEnabled(
		true
	);
	this->setSelectionMode(
		ExtendedSelection
	);
	this->header()->setDefaultSectionSize(
		150
	);
	// QAbstractItemView::startDrag() uses this property as the default drag action
	this->setDefaultDropAction(
		Qt::MoveAction
	);
	this->connect(
		this,
		&EntryView::doubleClicked,
		this,
		&EntryView::do_emitEntryActivated
	);
	this->connect(
		this->selectionModel(),
		&QItemSelectionModel::selectionChanged,
		this,
		&EntryView::sig_entrySelectionChanged
	);
	this->connect(
		this->model,
		&EntryModel::sig_switchedToEntryListMode,
		this,
		&EntryView::do_switchToEntryListMode
	);
	this->connect(
		this->model,
		&EntryModel::sig_switchedToGroupMode,
		this,
		&EntryView::do_switchToGroupMode
	);
}

void EntryView::keyPressEvent(
	QKeyEvent* event
)
{
	if((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) && this
		->currentIndex().isValid())
	{
		this->do_emitEntryActivated(
			this->currentIndex()
		);
	}
	QTreeView::keyPressEvent(
		event
	);
}

void EntryView::do_setGroup(
	Group* group
)
{
	this->model->do_setGroup(
		group
	);
	this->setFirstEntryActive();
}

void EntryView::setEntryList(
	const QList<Entry*> &entries
)
{
	this->model->setEntryList(
		entries
	);
	this->setFirstEntryActive();
}

void EntryView::setFirstEntryActive()
{
	if(this->model->rowCount() > 0)
	{
		const QModelIndex index_ = this->sortModel->mapToSource(
			this->sortModel->index(
				0,
				0
			)
		);
		this->setCurrentEntry(
			this->model->entryFromIndex(
				index_
			)
		);
	}
	else
	{
		 this->sig_entrySelectionChanged();
	}
}

bool EntryView::isInEntryListMode() const
{
	return this->inEntryListMode;
}

void EntryView::do_emitEntryActivated(
	const QModelIndex &index
)
{
	Entry* entry_ = this->entryFromIndex(
		index
	);
	 this->sig_entryActivated(
		entry_,
		static_cast<EntryModel::ModelColumn>(this->sortModel->mapToSource(
			index
		).column())
	);
}

void EntryView::setModel(
	QAbstractItemModel* model
)
{
	Q_UNUSED(
		model
	);
}

Entry* EntryView::getCurrentEntry() const
{
	if(QModelIndexList list_ = this->selectionModel()->selectedRows();
		list_.size() == 1)
	{
		return this->model->entryFromIndex(
			this->sortModel->mapToSource(
				list_.first()
			)
		);
	}
	return nullptr;
}

qsizetype EntryView::numberOfSelectedEntries() const
{
	return this->selectionModel()->selectedRows().size();
}

void EntryView::setCurrentEntry(
	Entry* entry
) const
{
	this->selectionModel()->setCurrentIndex(
		this->sortModel->mapFromSource(
			this->model->indexFromEntry(
				entry
			)
		),
		QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
	);
}

Entry* EntryView::entryFromIndex(
	const QModelIndex &index
) const
{
	if(index.isValid())
	{
		return this->model->entryFromIndex(
			this->sortModel->mapToSource(
				index
			)
		);
	}
	return nullptr;
}

void EntryView::do_switchToEntryListMode()
{
	this->sortModel->hideColumn(
		0,
		false
	);
	this->sortByColumn(
		1,
		Qt::AscendingOrder
	); // TODO: should probably be improved
	this->sortByColumn(
		0,
		Qt::AscendingOrder
	);
	this->inEntryListMode = true;
}

void EntryView::do_switchToGroupMode()
{
	this->sortModel->hideColumn(
		0,
		true
	);
	this->sortByColumn(
		-1,
		Qt::AscendingOrder
	);
	this->sortByColumn(
		0,
		Qt::AscendingOrder
	);
	this->inEntryListMode = false;
}

void EntryView::do_setFocus()
{
	this->setFocus();
}
