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
#include "EditWidget.h"
#include "ui_EditWidget.h"

EditWidget::EditWidget(
	QWidget* parent
)
	: DialogWidget(
		parent
	),
	ui(
		new Ui::EditWidget()
	)
{
	this->ui->setupUi(
		this
	);
	this->setReadOnly(
		false
	);
	QFont headerLabelFont_ = this->ui->headerLabel->font();
	headerLabelFont_.setBold(
		true
	);
	headerLabelFont_.setPointSize(
		headerLabelFont_.pointSize() + 2
	);
	this->headlineLabel()->setFont(
		headerLabelFont_
	);
	this->connect(
		this->ui->categoryList,
		&CategoryListWidget::currentRowChanged,
		this->ui->stackedWidget,
		&QStackedWidget::setCurrentIndex
	);
	this->connect(
		this->ui->buttonBox,
		&QDialogButtonBox::accepted,
		this,
		&EditWidget::sig_accepted
	);
	this->connect(
		this->ui->buttonBox,
		&QDialogButtonBox::rejected,
		this,
		&EditWidget::sig_rejected
	);
}

EditWidget::~EditWidget()
{
}

void EditWidget::add(
	const QString &labelText,
	QWidget* widget
) const
{
	this->ui->categoryList->addItem(
		labelText
	);
	this->ui->stackedWidget->addWidget(
		widget
	);
}

void EditWidget::setRowHidden(
	const QWidget* widget,
	const bool hide
) const
{
	if(const int row_ = this->ui->stackedWidget->indexOf(
			widget
		);
		row_ != -1)
	{
		this->ui->categoryList->item(
			row_
		)->setHidden(
			hide
		);
	}
}

void EditWidget::setCurrentRow(
	const int index
) const
{
	this->ui->categoryList->setCurrentRow(
		index
	);
}

void EditWidget::setHeadline(
	const QString &text
) const
{
	this->ui->headerLabel->setText(
		text
	);
}

QLabel* EditWidget::headlineLabel() const
{
	return this->ui->headerLabel;
}

void EditWidget::setReadOnly(
	const bool readOnly
)
{
	this->readOnly = readOnly;
	if(readOnly)
	{
		this->ui->buttonBox->setStandardButtons(
			QDialogButtonBox::Close
		);
	}
	else
	{
		this->ui->buttonBox->setStandardButtons(
			QDialogButtonBox::Ok | QDialogButtonBox::Cancel
		);
	}
}

bool EditWidget::isReadOnly() const
{
	return readOnly;
}
