/*
 *  Copyright (C) 2014 Felix Geyer <debfx@fobos.de>
 *  Copyright (C) 2014 Florian Geyer <blueice@fobos.de>
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
#include "DatabaseWidgetStateSync.h"
#include <gui/DatabaseTabWidget.h>
#include "core/Config.h"

DatabaseWidgetStateSync::DatabaseWidgetStateSync(
	QObject* parent
)
	: QObject(
		parent
	),
	activeDbWidget(
		nullptr
	),
	blockUpdates(
		false
	)
{
	this->splitterSizes = this->variantToIntList(
		Config::getInstance()->get(
			"GUI/SplitterState"
		)
	);
	this->columnSizesList = this->variantToIntList(
		Config::getInstance()->get(
			"GUI/EntryListColumnSizes"
		)
	);
	this->columnSizesSearch = this->variantToIntList(
		Config::getInstance()->get(
			"GUI/EntrySearchColumnSizes"
		)
	);
}

DatabaseWidgetStateSync::~DatabaseWidgetStateSync()
{
	Config::getInstance()->set(
		"GUI/SplitterState",
		this->intListToVariant(
			this->splitterSizes
		)
	);
	Config::getInstance()->set(
		"GUI/EntryListColumnSizes",
		this->intListToVariant(
			this->columnSizesList
		)
	);
	Config::getInstance()->set(
		"GUI/EntrySearchColumnSizes",
		this->intListToVariant(
			this->columnSizesSearch
		)
	);
}

void DatabaseWidgetStateSync::do_setActive(
	DatabaseWidget* dbWidget
)
{
	if(this->activeDbWidget)
	{
		this->disconnect(
			this->activeDbWidget,
			nullptr,
			this,
			nullptr
		);
	}
	this->activeDbWidget = dbWidget;
	if(this->activeDbWidget)
	{
		this->blockUpdates = true;
		if(!this->splitterSizes.isEmpty())
		{
			this->activeDbWidget->setSplitterSizes(
				this->splitterSizes
			);
		}
		if(this->activeDbWidget->isGroupSelected())
		{
			this->do_restoreListView();
		}
		else
		{
			this->do_restoreSearchView();
		}
		this->blockUpdates = false;
		this->connect(
			this->activeDbWidget,
			&DatabaseWidget::sig_splitterSizesChanged,
			this,
			&DatabaseWidgetStateSync::do_updateSplitterSizes
		);
		this->connect(
			this->activeDbWidget,
			&DatabaseWidget::sig_entryColumnSizesChanged,
			this,
			&DatabaseWidgetStateSync::do_updateColumnSizes
		);
		this->connect(
			this->activeDbWidget,
			&DatabaseWidget::sig_listModeActivated,
			this,
			&DatabaseWidgetStateSync::do_restoreListView
		);
		this->connect(
			this->activeDbWidget,
			&DatabaseWidget::sig_searchModeActivated,
			this,
			&DatabaseWidgetStateSync::do_restoreSearchView
		);
		this->connect(
			this->activeDbWidget,
			&DatabaseWidget::sig_listModeAboutToActivate,
			this,
			&DatabaseWidgetStateSync::do_getBlockUpdates
		);
		this->connect(
			this->activeDbWidget,
			&DatabaseWidget::sig_searchModeAboutToActivate,
			this,
			&DatabaseWidgetStateSync::do_getBlockUpdates
		);
	}
}

void DatabaseWidgetStateSync::do_restoreListView()
{
	if(!this->columnSizesList.isEmpty())
	{
		this->activeDbWidget->setEntryViewHeaderSizes(
			this->columnSizesList
		);
	}
	this->blockUpdates = false;
}

void DatabaseWidgetStateSync::do_restoreSearchView()
{
	if(!this->columnSizesSearch.isEmpty())
	{
		this->activeDbWidget->setEntryViewHeaderSizes(
			this->columnSizesSearch
		);
	}
	this->blockUpdates = false;
}

void DatabaseWidgetStateSync::do_getBlockUpdates()
{
	this->blockUpdates = true;
}

void DatabaseWidgetStateSync::do_updateSplitterSizes()
{
	if(this->blockUpdates)
	{
		return;
	}
	this->splitterSizes = this->activeDbWidget->getSplitterSizes();
}

void DatabaseWidgetStateSync::do_updateColumnSizes()
{
	if(this->blockUpdates)
	{
		return;
	}
	if(this->activeDbWidget->isGroupSelected())
	{
		this->columnSizesList = this->activeDbWidget->getEntryHeaderViewSizes();
	}
	else
	{
		this->columnSizesSearch = this->activeDbWidget->
			getEntryHeaderViewSizes();
	}
}

QList<int> DatabaseWidgetStateSync::variantToIntList(
	const QVariant &variant
)
{
	const QVariantList list_ = variant.toList();
	QList<int> result_;
	for(const QVariant &var_: list_)
	{
		bool ok_;
		const int size_ = var_.toInt(
			&ok_
		);
		if(ok_)
		{
			result_.append(
				size_
			);
		}
		else
		{
			result_.clear();
			break;
		}
	}
	return result_;
}

QVariant DatabaseWidgetStateSync::intListToVariant(
	const QList<int> &list
)
{
	QVariantList result_;
	for(const int value_: list)
	{
		result_.append(
			value_
		);
	}
	return result_;
}
