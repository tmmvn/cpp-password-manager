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
#include "SettingsWidget.h"
#include "ui_SettingsWidgetGeneral.h"
#include "ui_SettingsWidgetSecurity.h"
#include "core/Config.h"
#include "core/Translator.h"

SettingsWidget::SettingsWidget(
	QWidget* parent
)
	: EditWidget(
		parent
	),
	secWidget(
		new QWidget()
	),
	generalWidget(
		new QWidget()
	),
	secUi(
		new Ui::SettingsWidgetSecurity()
	),
	generalUi(
		new Ui::SettingsWidgetGeneral()
	)
{
	this->setHeadline(
		this->tr(
			"Application Settings"
		)
	);
	this->secUi->setupUi(
		this->secWidget
	);
	this->generalUi->setupUi(
		this->generalWidget
	);
	this->add(
		this->tr(
			"General"
		),
		this->generalWidget
	);
	this->add(
		this->tr(
			"Security"
		),
		this->secWidget
	);
#ifdef Q_OS_MAC
	// systray not useful on OS X
	this->generalUi->systrayShowCheckBox->setVisible(
		false
	);
	this->generalUi->systrayMinimizeToTrayCheckBox->setVisible(
		false
	);
#endif
	this->connect(
		this,
		&EditWidget::sig_accepted,
		this,
		&SettingsWidget::do_saveSettings
	);
	this->connect(
		this,
		&EditWidget::sig_rejected,
		this,
		&SettingsWidget::do_reject
	);
	this->connect(
		this->generalUi->autoSaveAfterEveryChangeCheckBox,
		&QCheckBox::toggled,
		this,
		&SettingsWidget::do_enableAutoSaveOnExit
	);
	this->connect(
		this->generalUi->systrayShowCheckBox,
		&QCheckBox::toggled,
		this->generalUi->systrayMinimizeToTrayCheckBox,
		&QCheckBox::setEnabled
	);
	this->connect(
		this->secUi->clearClipboardCheckBox,
		&QCheckBox::toggled,
		this->secUi->clearClipboardSpinBox,
		&QSpinBox::setEnabled
	);
	this->connect(
		this->secUi->lockDatabaseIdleCheckBox,
		&QCheckBox::toggled,
		this->secUi->lockDatabaseIdleSpinBox,
		&QSpinBox::setEnabled
	);
}

SettingsWidget::~SettingsWidget()
{
}

void SettingsWidget::loadSettings()
{
	this->generalUi->rememberLastDatabasesCheckBox->setChecked(
		Config::getInstance()->get(
			"RememberLastDatabases"
		).toBool()
	);
	this->generalUi->rememberLastKeyFilesCheckBox->setChecked(
		Config::getInstance()->get(
			"RememberLastKeyFiles"
		).toBool()
	);
	this->generalUi->openPreviousDatabasesOnStartupCheckBox->setChecked(
		Config::getInstance()->get(
			"OpenPreviousDatabasesOnStartup"
		).toBool()
	);
	this->generalUi->autoSaveAfterEveryChangeCheckBox->setChecked(
		Config::getInstance()->get(
			"AutoSaveAfterEveryChange"
		).toBool()
	);
	this->generalUi->autoSaveOnExitCheckBox->setChecked(
		Config::getInstance()->get(
			"AutoSaveOnExit"
		).toBool()
	);
	this->generalUi->minimizeOnCopyCheckBox->setChecked(
		Config::getInstance()->get(
			"MinimizeOnCopy"
		).toBool()
	);
	this->generalUi->useGroupIconOnEntryCreationCheckBox->setChecked(
		Config::getInstance()->get(
			"UseGroupIconOnEntryCreation"
		).toBool()
	);
	this->generalUi->languageComboBox->clear();
	QList<QPair<QString, QString>> languages_ =
		Translator::availableLanguages();
	for(auto i_ = 0; i_ < languages_.size(); i_++)
	{
		this->generalUi->languageComboBox->addItem(
			languages_[i_].second,
			languages_[i_].first
		);
	}
	if(const int defaultIndex_ = this->generalUi->languageComboBox->findData(
			Config::getInstance()->get(
				"GUI/Language"
			)
		);
		defaultIndex_ > 0)
	{
		this->generalUi->languageComboBox->setCurrentIndex(
			defaultIndex_
		);
	}
	this->generalUi->systrayShowCheckBox->setChecked(
		Config::getInstance()->get(
			"GUI/ShowTrayIcon"
		).toBool()
	);
	this->generalUi->systrayMinimizeToTrayCheckBox->setChecked(
		Config::getInstance()->get(
			"GUI/MinimizeToTray"
		).toBool()
	);
	this->secUi->clearClipboardCheckBox->setChecked(
		Config::getInstance()->get(
			"security/clearclipboard"
		).toBool()
	);
	this->secUi->clearClipboardSpinBox->setValue(
		Config::getInstance()->get(
			"security/clearclipboardtimeout"
		).toInt()
	);
	this->secUi->lockDatabaseIdleCheckBox->setChecked(
		Config::getInstance()->get(
			"security/lockdatabaseidle"
		).toBool()
	);
	this->secUi->lockDatabaseIdleSpinBox->setValue(
		Config::getInstance()->get(
			"security/lockdatabaseidlesec"
		).toInt()
	);
	this->secUi->passwordCleartextCheckBox->setChecked(
		Config::getInstance()->get(
			"security/passwordscleartext"
		).toBool()
	);
	this->secUi->autoTypeAskCheckBox->setChecked(
		Config::getInstance()->get(
			"security/autotypeask"
		).toBool()
	);
	this->setCurrentRow(
		0
	);
}

void SettingsWidget::do_saveSettings()
{
	Config::getInstance()->set(
		"RememberLastDatabases",
		this->generalUi->rememberLastDatabasesCheckBox->isChecked()
	);
	Config::getInstance()->set(
		"RememberLastKeyFiles",
		this->generalUi->rememberLastKeyFilesCheckBox->isChecked()
	);
	Config::getInstance()->set(
		"OpenPreviousDatabasesOnStartup",
		this->generalUi->openPreviousDatabasesOnStartupCheckBox->isChecked()
	);
	Config::getInstance()->set(
		"AutoSaveAfterEveryChange",
		this->generalUi->autoSaveAfterEveryChangeCheckBox->isChecked()
	);
	Config::getInstance()->set(
		"AutoSaveOnExit",
		this->generalUi->autoSaveOnExitCheckBox->isChecked()
	);
	Config::getInstance()->set(
		"MinimizeOnCopy",
		this->generalUi->minimizeOnCopyCheckBox->isChecked()
	);
	Config::getInstance()->set(
		"UseGroupIconOnEntryCreation",
		this->generalUi->useGroupIconOnEntryCreationCheckBox->isChecked()
	);
	const int currentLangIndex_ = this->generalUi->languageComboBox->
		currentIndex();
	Config::getInstance()->set(
		"GUI/Language",
		this->generalUi->languageComboBox->itemData(
			currentLangIndex_
		).toString()
	);
	Config::getInstance()->set(
		"GUI/ShowTrayIcon",
		this->generalUi->systrayShowCheckBox->isChecked()
	);
	Config::getInstance()->set(
		"GUI/MinimizeToTray",
		this->generalUi->systrayMinimizeToTrayCheckBox->isChecked()
	);
	Config::getInstance()->set(
		"security/clearclipboard",
		this->secUi->clearClipboardCheckBox->isChecked()
	);
	Config::getInstance()->set(
		"security/clearclipboardtimeout",
		this->secUi->clearClipboardSpinBox->value()
	);
	Config::getInstance()->set(
		"security/lockdatabaseidle",
		this->secUi->lockDatabaseIdleCheckBox->isChecked()
	);
	Config::getInstance()->set(
		"security/lockdatabaseidlesec",
		this->secUi->lockDatabaseIdleSpinBox->value()
	);
	Config::getInstance()->set(
		"security/passwordscleartext",
		this->secUi->passwordCleartextCheckBox->isChecked()
	);
	Config::getInstance()->set(
		"security/autotypeask",
		this->secUi->autoTypeAskCheckBox->isChecked()
	);
	this->sig_editFinished(
		true
	);
}

void SettingsWidget::do_reject()
{
	this->sig_editFinished(
		false
	);
}

void SettingsWidget::do_enableAutoSaveOnExit(
	const bool checked
) const
{
	this->generalUi->autoSaveOnExitCheckBox->setEnabled(
		!checked
	);
}
