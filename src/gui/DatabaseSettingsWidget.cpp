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
#include "DatabaseSettingsWidget.h"
#include "ui_DatabaseSettingsWidget.h"
#include "core/Database.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "keys/CompositeKey.h"

DatabaseSettingsWidget::DatabaseSettingsWidget(
	QWidget* parent
)
	: DialogWidget(
		parent
	),
	ui(
		new Ui::DatabaseSettingsWidget()
	),
	db(
		nullptr
	)
{
	this->ui->setupUi(
		this
	);
	this->connect(
		this->ui->buttonBox,
		&QDialogButtonBox::accepted,
		this,
		&DatabaseSettingsWidget::do_save
	);
	this->connect(
		this->ui->buttonBox,
		&QDialogButtonBox::rejected,
		this,
		&DatabaseSettingsWidget::do_reject
	);
	this->connect(
		this->ui->historyMaxItemsCheckBox,
		&QCheckBox::toggled,
		this->ui->historyMaxItemsSpinBox,
		&QSpinBox::setEnabled
	);
	this->connect(
		this->ui->historyMaxSizeCheckBox,
		&QCheckBox::toggled,
		this->ui->historyMaxSizeSpinBox,
		&QSpinBox::setEnabled
	);
	this->connect(
		this->ui->transformBenchmarkButton,
		&QPushButton::clicked,
		this,
		&DatabaseSettingsWidget::do_transformRoundsBenchmark
	);
}

DatabaseSettingsWidget::~DatabaseSettingsWidget()
{
}

void DatabaseSettingsWidget::load(
	Database* db
)
{
	this->db = db;
	const Metadata* meta_ = this->db->getMetadata();
	this->ui->dbNameEdit->setText(
		meta_->getName()
	);
	this->ui->dbDescriptionEdit->setText(
		meta_->getDescription()
	);
	this->ui->recycleBinEnabledCheckBox->setChecked(
		meta_->recycleBinEnabled()
	);
	this->ui->defaultUsernameEdit->setText(
		meta_->getDefaultUserName()
	);
	this->ui->transformRoundsSpinBox->setValue(
		static_cast<int>(this->db->transformRounds())
	);
	if(meta_->getHistoryMaxItems() > -1)
	{
		this->ui->historyMaxItemsSpinBox->setValue(
			meta_->getHistoryMaxItems()
		);
		this->ui->historyMaxItemsCheckBox->setChecked(
			true
		);
	}
	else
	{
		this->ui->historyMaxItemsSpinBox->setValue(
			Metadata::DefaultHistoryMaxItems
		);
		this->ui->historyMaxItemsCheckBox->setChecked(
			false
		);
	}
	if(const int historyMaxSizeMiB_ = qRound(
			meta_->getHistoryMaxSize() / static_cast<qreal>(1048576)
		);
		historyMaxSizeMiB_ > 0)
	{
		this->ui->historyMaxSizeSpinBox->setValue(
			historyMaxSizeMiB_
		);
		this->ui->historyMaxSizeCheckBox->setChecked(
			true
		);
	}
	else
	{
		this->ui->historyMaxSizeSpinBox->setValue(
			Metadata::DefaultHistoryMaxSize
		);
		this->ui->historyMaxSizeCheckBox->setChecked(
			false
		);
	}
	this->ui->dbNameEdit->setFocus();
}

void DatabaseSettingsWidget::do_save()
{
	Metadata* meta_ = this->db->getMetadata();
	meta_->setName(
		this->ui->dbNameEdit->text()
	);
	meta_->setDescription(
		this->ui->dbDescriptionEdit->text()
	);
	meta_->setDefaultUserName(
		this->ui->defaultUsernameEdit->text()
	);
	meta_->setRecycleBinEnabled(
		this->ui->recycleBinEnabledCheckBox->isChecked()
	);
	if(static_cast<quint64>(this->ui->transformRoundsSpinBox->value()) != db->
		transformRounds())
	{
		QApplication::setOverrideCursor(
			QCursor(
				Qt::WaitCursor
			)
		);
		this->db->setTransformRounds(
			this->ui->transformRoundsSpinBox->value()
		);
		QApplication::restoreOverrideCursor();
	}
	auto truncate_ = false;
	int historyMaxItems_;
	if(this->ui->historyMaxItemsCheckBox->isChecked())
	{
		historyMaxItems_ = this->ui->historyMaxItemsSpinBox->value();
	}
	else
	{
		historyMaxItems_ = -1;
	}
	if(historyMaxItems_ != meta_->getHistoryMaxItems())
	{
		meta_->setHistoryMaxItems(
			historyMaxItems_
		);
		truncate_ = true;
	}
	int historyMaxSize_;
	if(this->ui->historyMaxSizeCheckBox->isChecked())
	{
		historyMaxSize_ = this->ui->historyMaxSizeSpinBox->value() * 1048576;
	}
	else
	{
		historyMaxSize_ = -1;
	}
	if(historyMaxSize_ != meta_->getHistoryMaxSize())
	{
		meta_->setHistoryMaxSize(
			historyMaxSize_
		);
		truncate_ = true;
	}
	if(truncate_)
	{
		this->truncateHistories();
	}
	 this->sig_editFinished(
		true
	);
}

void DatabaseSettingsWidget::do_reject()
{
	 this->sig_editFinished(
		false
	);
}

void DatabaseSettingsWidget::do_transformRoundsBenchmark() const
{
	QApplication::setOverrideCursor(
		QCursor(
			Qt::WaitCursor
		)
	);
	if(const int rounds_ = CompositeKey::transformKeyBenchmark(
			1000
		);
		rounds_ != -1)
	{
		this->ui->transformRoundsSpinBox->setValue(
			rounds_
		);
	}
	QApplication::restoreOverrideCursor();
}

void DatabaseSettingsWidget::truncateHistories() const
{
	const QList<Entry*> allEntries_ = this->db->getRootGroup()->
		getEntriesRecursive(
			false
		);
	for(Entry* entry_: allEntries_)
	{
		entry_->truncateHistory();
	}
}
