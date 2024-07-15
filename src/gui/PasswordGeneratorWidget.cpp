/*
 *  Copyright (C) 2013 Felix Geyer <debfx@fobos.de>
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
#include "PasswordGeneratorWidget.h"
#include <QLineEdit>
#include "ui_PasswordGeneratorWidget.h"
#include "core/Config.h"
#include "core/FilePath.h"
#include "core/PasswordGenerator.h"

PasswordGeneratorWidget::PasswordGeneratorWidget(
	QWidget* parent
)
	: QWidget(
		parent
	),
	updatingSpinBox(
		false
	),
	generator(
		new PasswordGenerator()
	),
	ui(
		new Ui::PasswordGeneratorWidget()
	)
{
	this->ui->setupUi(
		this
	);
	this->ui->togglePasswordButton->setIcon(
		FilePath::getInstance()->getOnOffIcon(
			"actions",
			"password-show"
		)
	);
	this->connect(
		this->ui->editNewPassword->lineEdit(),
		&QLineEdit::textChanged,
		this,
		&PasswordGeneratorWidget::do_updateApplyEnabled
	);
	this->connect(
		this->ui->togglePasswordButton,
		&QToolButton::toggled,
		this->ui->editNewPassword,
		&PasswordComboBox::do_setEcho
	);
	this->connect(
		this->ui->buttonApply,
		&QPushButton::clicked,
		this,
		&PasswordGeneratorWidget::do_emitNewPassword
	);
	this->connect(
		this->ui->buttonApply,
		&QPushButton::clicked,
		this,
		&PasswordGeneratorWidget::do_saveSettings
	);
	this->connect(
		this->ui->sliderLength,
		&QSlider::valueChanged,
		this,
		&PasswordGeneratorWidget::do_sliderMoved
	);
	this->connect(
		this->ui->spinBoxLength,
		&QSpinBox::valueChanged,
		this,
		&PasswordGeneratorWidget::do_spinBoxChanged
	);
	this->connect(
		this->ui->optionButtons,
		&QButtonGroup::idClicked,
		this,
		&PasswordGeneratorWidget::do_updateGenerator
	);
	this->ui->editNewPassword->setGenerator(
		this->generator.data()
	);
	this->loadSettings();
	this->reset();
}

PasswordGeneratorWidget::~PasswordGeneratorWidget()
{
}

void PasswordGeneratorWidget::loadSettings() const
{
	this->ui->checkBoxLower->setChecked(
		Config::getInstance()->get(
			"generator/LowerCase",
			true
		).toBool()
	);
	this->ui->checkBoxUpper->setChecked(
		Config::getInstance()->get(
			"generator/UpperCase",
			true
		).toBool()
	);
	this->ui->checkBoxNumbers->setChecked(
		Config::getInstance()->get(
			"generator/Numbers",
			true
		).toBool()
	);
	this->ui->checkBoxSpecialChars->setChecked(
		Config::getInstance()->get(
			"generator/SpecialChars",
			false
		).toBool()
	);
	this->ui->checkBoxExcludeAlike->setChecked(
		Config::getInstance()->get(
			"generator/ExcludeAlike",
			true
		).toBool()
	);
	this->ui->checkBoxEnsureEvery->setChecked(
		Config::getInstance()->get(
			"generator/EnsureEvery",
			true
		).toBool()
	);
	this->ui->spinBoxLength->setValue(
		Config::getInstance()->get(
			"generator/Length",
			16
		).toInt()
	);
}

void PasswordGeneratorWidget::do_saveSettings() const
{
	Config::getInstance()->set(
		"generator/LowerCase",
		this->ui->checkBoxLower->isChecked()
	);
	Config::getInstance()->set(
		"generator/UpperCase",
		this->ui->checkBoxUpper->isChecked()
	);
	Config::getInstance()->set(
		"generator/Numbers",
		this->ui->checkBoxNumbers->isChecked()
	);
	Config::getInstance()->set(
		"generator/SpecialChars",
		this->ui->checkBoxSpecialChars->isChecked()
	);
	Config::getInstance()->set(
		"generator/ExcludeAlike",
		this->ui->checkBoxExcludeAlike->isChecked()
	);
	Config::getInstance()->set(
		"generator/EnsureEvery",
		this->ui->checkBoxEnsureEvery->isChecked()
	);
	Config::getInstance()->set(
		"generator/Length",
		this->ui->spinBoxLength->value()
	);
}

void PasswordGeneratorWidget::reset()
{
	this->ui->editNewPassword->lineEdit()->setText(
		""
	);
	this->ui->togglePasswordButton->setChecked(
		Config::getInstance()->get(
			"security/passwordscleartext"
		).toBool()
	);
	this->do_updateGenerator();
}

void PasswordGeneratorWidget::regeneratePassword() const
{
	if(this->generator->isValid())
	{
		const QString password_ = this->generator->generatePassword();
		this->ui->editNewPassword->setEditText(
			password_
		);
	}
}

void PasswordGeneratorWidget::do_updateApplyEnabled(
	const QString &password
) const
{
	this->ui->buttonApply->setEnabled(
		!password.isEmpty()
	);
}

void PasswordGeneratorWidget::do_emitNewPassword()
{
	this->sig_newPassword(
		this->ui->editNewPassword->lineEdit()->text()
	);
}

void PasswordGeneratorWidget::do_sliderMoved()
{
	if(this->updatingSpinBox)
	{
		return;
	}
	this->ui->spinBoxLength->setValue(
		this->ui->sliderLength->value()
	);
	this->do_updateGenerator();
}

void PasswordGeneratorWidget::do_spinBoxChanged()
{
	if(this->updatingSpinBox)
	{
		return;
	}
	// Interlock so that we don't update twice - this causes issues as the spinbox can go higher than slider
	this->updatingSpinBox = true;
	this->ui->sliderLength->setValue(
		this->ui->spinBoxLength->value()
	);
	this->updatingSpinBox = false;
	this->do_updateGenerator();
}

PasswordGenerator::CharClasses PasswordGeneratorWidget::charClasses() const
{
	PasswordGenerator::CharClasses classes_;
	if(this->ui->checkBoxLower->isChecked())
	{
		classes_ |= PasswordGenerator::LowerLetters;
	}
	if(this->ui->checkBoxUpper->isChecked())
	{
		classes_ |= PasswordGenerator::UpperLetters;
	}
	if(this->ui->checkBoxNumbers->isChecked())
	{
		classes_ |= PasswordGenerator::Numbers;
	}
	if(this->ui->checkBoxSpecialChars->isChecked())
	{
		classes_ |= PasswordGenerator::SpecialCharacters;
	}
	return classes_;
}

PasswordGenerator::GeneratorFlags
PasswordGeneratorWidget::generatorFlags() const
{
	PasswordGenerator::GeneratorFlags flags_;
	if(this->ui->checkBoxExcludeAlike->isChecked())
	{
		flags_ |= PasswordGenerator::ExcludeLookAlike;
	}
	if(this->ui->checkBoxEnsureEvery->isChecked())
	{
		flags_ |= PasswordGenerator::CharFromEveryGroup;
	}
	return flags_;
}

void PasswordGeneratorWidget::do_updateGenerator()
{
	const PasswordGenerator::CharClasses classes_ = this->charClasses();
	const PasswordGenerator::GeneratorFlags flags_ = this->generatorFlags();
	auto minLength_ = 0;
	if(flags_.testFlag(
		PasswordGenerator::CharFromEveryGroup
	))
	{
		if(classes_.testFlag(
			PasswordGenerator::LowerLetters
		))
		{
			minLength_++;
		}
		if(classes_.testFlag(
			PasswordGenerator::UpperLetters
		))
		{
			minLength_++;
		}
		if(classes_.testFlag(
			PasswordGenerator::Numbers
		))
		{
			minLength_++;
		}
		if(classes_.testFlag(
			PasswordGenerator::SpecialCharacters
		))
		{
			minLength_++;
		}
	}
	minLength_ = qMax(
		minLength_,
		1
	);
	if(this->ui->spinBoxLength->value() < minLength_)
	{
		this->updatingSpinBox = true;
		this->ui->spinBoxLength->setValue(
			minLength_
		);
		this->ui->sliderLength->setValue(
			minLength_
		);
		this->updatingSpinBox = false;
	}
	this->ui->spinBoxLength->setMinimum(
		minLength_
	);
	this->ui->sliderLength->setMinimum(
		minLength_
	);
	this->generator->setLength(
		this->ui->spinBoxLength->value()
	);
	this->generator->setCharClasses(
		classes_
	);
	this->generator->setFlags(
		flags_
	);
	this->regeneratePassword();
}
