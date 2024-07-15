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
#ifndef KEEPASSX_EDITENTRYWIDGET_P_H
#define KEEPASSX_EDITENTRYWIDGET_P_H
#include <QListWidget>
#include <QScrollBar>
#include <QSize>
#include <QStyledItemDelegate>

class CategoryListViewDelegate final:public QStyledItemDelegate
{
public:
	explicit CategoryListViewDelegate(
		QObject* parent
	)
		: QStyledItemDelegate(
			parent
		)
	{
	}

	virtual QSize sizeHint(
		const QStyleOptionViewItem &option,
		const QModelIndex &index
	) const override
	{
		QSize size_ = QStyledItemDelegate::sizeHint(
			option,
			index
		);
		size_.setHeight(
			qMax(
				size_.height(),
				22
			)
		);
		return size_;
	}
};

class CategoryListWidget final:public QListWidget
{
public:
	explicit CategoryListWidget(
		QWidget* parent = nullptr
	)
		: QListWidget(
			parent
		)
	{
		this->setSizePolicy(
			QSizePolicy::Fixed,
			QSizePolicy::Expanding
		);
		this->setItemDelegate(
			new CategoryListViewDelegate(
				this
			)
		);
	}

	virtual QSize sizeHint() const override
	{
		QSize sizeHint_ = QListWidget::sizeHint();
		int width_ = this->sizeHintForColumn(
			0
		) + frameWidth() * 2 + 5;
		if(this->verticalScrollBar()->isVisible())
		{
			width_ += this->verticalScrollBar()->width();
		}
		sizeHint_.setWidth(
			width_
		);
		return sizeHint_;
	}
};

class AttributesListView final:public QListView
{
public:
	explicit AttributesListView(
		QWidget* parent = nullptr
	)
		: QListView(
			parent
		)
	{
		this->setSizePolicy(
			QSizePolicy::Preferred,
			QSizePolicy::Expanding
		);
		this->setItemDelegate(
			new CategoryListViewDelegate(
				this
			)
		);
	}

	virtual QSize sizeHint() const override
	{
		QSize sizeHint_ = QListView::sizeHint();
		int width_ = this->sizeHintForColumn(
			0
		) + frameWidth() * 2 + 5;
		if(this->verticalScrollBar()->isVisible())
		{
			width_ += this->verticalScrollBar()->width();
		}
		sizeHint_.setWidth(
			width_
		);
		return sizeHint_;
	}
};
#endif // KEEPASSX_EDITENTRYWIDGET_P_H
