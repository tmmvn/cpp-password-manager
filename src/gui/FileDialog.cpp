/*
 *  Copyright (C) 2011 Felix Geyer <debfx@fobos.de>
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
#include "FileDialog.h"
#include "core/Config.h"
FileDialog* FileDialog::instance(
	nullptr
);

QString FileDialog::getOpenFileName(
	QWidget* parent,
	const QString &caption,
	QString dir,
	const QString &filter,
	QString* selectedFilter
)
{
	if(!this->nextFileName.isEmpty())
	{
		QString result_ = this->nextFileName;
		this->nextFileName = "";
		return result_;
	}
	if(dir.isEmpty())
	{
		dir = Config::getInstance()->get(
			"LastDir"
		).toString();
	}
	QString result_ = QFileDialog::getOpenFileName(
		parent,
		caption,
		dir,
		filter,
		selectedFilter
	);
	// on Mac OS X the focus is lost after closing the native dialog
	if(parent)
	{
		parent->activateWindow();
	}
	if(!result_.isEmpty())
	{
		Config::getInstance()->set(
			"LastDir",
			QFileInfo(
				result_
			).absolutePath()
		);
	}
	return result_;
}

QString FileDialog::getSaveFileName(
	QWidget* parent,
	const QString &caption,
	QString dir,
	const QString &filter,
	QString* selectedFilter,
	const QString &defaultExtension
)
{
	if(!this->nextFileName.isEmpty())
	{
		QString result_ = this->nextFileName;
		this->nextFileName = "";
		return result_;
	}
	if(dir.isEmpty())
	{
		dir = Config::getInstance()->get(
			"LastDir"
		).toString();
	}
	QString result_;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
	Q_UNUSED(
		defaultExtension
	);
	// the native dialogs on these platforms already append the file extension
	result_ = QFileDialog::getSaveFileName(
		parent,
		caption,
		dir,
		filter,
		selectedFilter
	);
#else
	QFileDialog dialog_(parent, caption, dir, filter);
	dialog_.setAcceptMode(QFileDialog::AcceptSave);
	dialog_.setFileMode(QFileDialog::AnyFile);
	if (selectedFilter) {
	    dialog_.selectNameFilter(*selectedFilter);
	}
	dialog_.setOptions(this->options);
	dialog_.setDefaultSuffix(defaultExtension);
	QStringList results_;
	if (dialog_.exec()) {
	    results_ = dialog_.selectedFiles();
	    if (!results_.isEmpty()) {
	        result_ = results_[0];
	    }
	}
#endif
	// on Mac OS X the focus is lost after closing the native dialog
	if(parent)
	{
		parent->activateWindow();
	}
	if(!result_.isEmpty())
	{
		Config::getInstance()->set(
			"LastDir",
			QFileInfo(
				result_
			).absolutePath()
		);
	}
	return result_;
}

void FileDialog::setNextFileName(
	const QString &fileName
)
{
	this->nextFileName = fileName;
}

FileDialog::FileDialog()
{
}

FileDialog* FileDialog::getInstance()
{
	if(!instance)
	{
		instance = new FileDialog();
	}
	return instance;
}
