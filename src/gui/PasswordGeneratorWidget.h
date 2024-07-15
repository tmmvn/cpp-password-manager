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
#ifndef KEEPASSX_PASSWORDGENERATORWIDGET_H
#define KEEPASSX_PASSWORDGENERATORWIDGET_H
#include <QWidget>
#include <QComboBox>
#include "core/PasswordGenerator.h"

namespace Ui
{
	class PasswordGeneratorWidget;
}

class PasswordGenerator;

class PasswordGeneratorWidget final:public QWidget
{
	Q_OBJECT public:
	explicit PasswordGeneratorWidget(
		QWidget* parent = nullptr
	);
	virtual ~PasswordGeneratorWidget() override;
	void loadSettings() const;
	void reset();
	void regeneratePassword() const;
Q_SIGNALS:
	void sig_newPassword(
		const QString &password
	);
private Q_SLOTS:
	void do_updateApplyEnabled(
		const QString &password
	) const;
	void do_emitNewPassword();
	void do_saveSettings() const;
	void do_sliderMoved();
	void do_spinBoxChanged();
	void do_updateGenerator();
private:
	bool updatingSpinBox;
	PasswordGenerator::CharClasses charClasses() const;
	PasswordGenerator::GeneratorFlags generatorFlags() const;
	const QScopedPointer<PasswordGenerator> generator;
	const QScopedPointer<Ui::PasswordGeneratorWidget> ui;
};
#endif // KEEPASSX_PASSWORDGENERATORWIDGET_H
