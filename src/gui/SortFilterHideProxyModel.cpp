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
#include "SortFilterHideProxyModel.h"

SortFilterHideProxyModel::SortFilterHideProxyModel(
	QObject* parent
)
	: QSortFilterProxyModel(
		parent
	)
{
}

Qt::DropActions SortFilterHideProxyModel::supportedDragActions() const
{
	return this->sourceModel()->supportedDragActions();
}

void SortFilterHideProxyModel::hideColumn(
	const int column,
	const bool hide
)
{
	this->hiddenColumns.resize(
		column + 1
	);
	this->hiddenColumns[column] = hide;
	this->invalidateFilter();
}

bool SortFilterHideProxyModel::filterAcceptsColumn(
	const int sourceColumn,
	const QModelIndex &sourceParent
) const
{
	Q_UNUSED(
		sourceParent
	);
	return sourceColumn >= this->hiddenColumns.size() || !this->hiddenColumns.
		at(
			sourceColumn
		);
}
