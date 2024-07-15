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
#include "AboutDialog.h"
#include "config-keepassx.h"
#include "ui_AboutDialog.h"
#include "version.h"
#include "core/FilePath.h"
#include "crypto/Crypto.h"

AboutDialog::AboutDialog(
	QWidget* parent
)
	: QDialog(
		parent
	),
	ui(
		new Ui::AboutDialog()
	)
{
	this->ui->setupUi(
		this
	);
	this->ui->nameLabel->setText(
		this->ui->nameLabel->text() + " " + KEEPASSX_VERSION
	);
	QFont nameLabelFont_ = this->ui->nameLabel->font();
	nameLabelFont_.setBold(
		true
	);
	nameLabelFont_.setPointSize(
		nameLabelFont_.pointSize() + 4
	);
	this->ui->nameLabel->setFont(
		nameLabelFont_
	);
	this->ui->iconLabel->setPixmap(
		FilePath::getInstance()->getApplicationIcon().pixmap(
			48
		)
	);
	QString commitHash_;
	if(!QString(
		GIT_HEAD
	).isEmpty())
	{
		commitHash_ = GIT_HEAD;
	}
	else if(!QString(
		DIST_HASH
	).contains(
		"Format"
	))
	{
		commitHash_ = DIST_HASH;
	}
	if(!commitHash_.isEmpty())
	{
		const QString labelText_ = tr(
			"Revision"
		).append(
			": "
		).append(
			commitHash_
		);
		this->ui->label_git->setText(
			labelText_
		);
	}
	const QString libs_ = QString(
		"%1\n- Qt %2\n- %3"
	).arg(
		this->ui->label_libs->text()
	).arg(
		QString::fromLocal8Bit(
			qVersion()
		)
	).arg(
		Crypto::getBackendVersion()
	);
	this->ui->label_libs->setText(
		libs_
	);
	this->setAttribute(
		Qt::WA_DeleteOnClose
	);
	this->connect(
		this->ui->buttonBox,
		SIGNAL(
			rejected()
		),
		SLOT(
			close()
		)
	);
}

AboutDialog::~AboutDialog()
{
}
