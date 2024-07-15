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
#include "GroupView.h"
#include <QDragMoveEvent>
#include <QMetaObject>
#include <QMimeData>
#include "core/Group.h"
#include "gui/group/GroupModel.h"

GroupView::GroupView(
	Database* db,
	QWidget* parent
)
	: QTreeView(
		parent
	),
	model(
		new GroupModel(
			db,
			this
		)
	),
	updatingExpanded(
		false
	)
{
	QTreeView::setModel(
		this->model
	);
	this->setHeaderHidden(
		true
	);
	this->setUniformRowHeights(
		true
	);
	this->connect(
		this,
		&GroupView::expanded,
		this,
		&GroupView::do_expandedChanged
	);
	this->connect(
		this,
		&GroupView::collapsed,
		this,
		&GroupView::do_expandedChanged
	);
	this->connect(
		this->model,
		&GroupModel::rowsInserted,
		this,
		&GroupView::do_syncExpandedState
	);
	this->connect(
		this->model,
		&GroupModel::modelReset,
		this,
		&GroupView::do_modelReset
	);
	this->connect(
		this->selectionModel(),
		&QItemSelectionModel::currentChanged,
		this,
		&GroupView::do_emitGroupChanged
	);
	this->do_modelReset();
	this->setDragEnabled(
		true
	);
	this->viewport()->setAcceptDrops(
		true
	);
	this->setDropIndicatorShown(
		true
	);
	this->setDefaultDropAction(
		Qt::MoveAction
	);
}

void GroupView::changeDatabase(
	Database* newDb
) const
{
	this->model->changeDatabase(
		newDb
	);
}

void GroupView::dragMoveEvent(
	QDragMoveEvent* event
)
{
	if(event->modifiers() & Qt::ControlModifier)
	{
		event->setDropAction(
			Qt::CopyAction
		);
	}
	else
	{
		event->setDropAction(
			Qt::MoveAction
		);
	}
	QTreeView::dragMoveEvent(
		event
	);
	// entries may only be dropped on groups
	if(event->isAccepted() && event->mimeData()->hasFormat(
		"application/x-keepassx-entry"
	) && (this->dropIndicatorPosition() == AboveItem || this->
		dropIndicatorPosition() == BelowItem))
	{
		event->ignore();
	}
}

Group* GroupView::getCurrentGroup() const
{
	if(this->currentIndex() == QModelIndex())
	{
		return nullptr;
	}
	return this->model->groupFromIndex(
		this->currentIndex()
	);
}

void GroupView::do_expandedChanged(
	const QModelIndex &index
) const
{
	if(this->updatingExpanded)
	{
		return;
	}
	Group* group_ = this->model->groupFromIndex(
		index
	);
	group_->setExpanded(
		this->isExpanded(
			index
		)
	);
}

void GroupView::recInitExpanded(
	Group* group
)
{
	this->updatingExpanded = true;
	this->expandGroup(
		group,
		group->isExpanded()
	);
	this->updatingExpanded = false;
	const QList<Group*> children_ = group->getChildren();
	for(Group* child_: children_)
	{
		this->recInitExpanded(
			child_
		);
	}
}

void GroupView::expandGroup(
	Group* group,
	const bool expand
)
{
	const QModelIndex index_ = this->model->index(
		group
	);
	this->setExpanded(
		index_,
		expand
	);
}

void GroupView::emitGroupChanged(
	const QModelIndex &index
)
{
	this->sig_groupChanged(
		this->model->groupFromIndex(
			index
		)
	);
}

void GroupView::setModel(
	QAbstractItemModel* model
)
{
	Q_UNUSED(
		model
	);
}

void GroupView::do_emitGroupChanged()
{
	this->sig_groupChanged(
		this->getCurrentGroup()
	);
}

void GroupView::do_syncExpandedState(
	const QModelIndex &parent,
	const int start,
	const int end
)
{
	for(int row_ = start; row_ <= end; row_++)
	{
		Group* group_ = this->model->groupFromIndex(
			this->model->index(
				row_,
				0,
				parent
			)
		);
		this->recInitExpanded(
			group_
		);
	}
}

void GroupView::setCurrentGroup(
	Group* group
)
{
	this->setCurrentIndex(
		this->model->index(
			group
		)
	);
}

void GroupView::do_modelReset()
{
	this->recInitExpanded(
		this->model->groupFromIndex(
			this->model->index(
				0,
				0
			)
		)
	);
	this->setCurrentIndex(
		this->model->index(
			0,
			0
		)
	);
}
