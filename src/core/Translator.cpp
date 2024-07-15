/*
 *  Copyright (C) 2014 Felix Geyer <debfx@fobos.de>
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
#include "Translator.h"
#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QTranslator>
#include "config-keepassx.h"
#include "core/Config.h"
#include "core/FilePath.h"

void Translator::installTranslator()
{
	QString language_ = Config::getInstance()->get(
		"GUI/Language"
	).toString();
	if(language_ == "system" || language_.isEmpty())
	{
		language_ = QLocale::system().name();
	}
	if(!installTranslator(
		language_
	))
	{
		// English fallback still needs translations for plurals
		if(!installTranslator(
			"en_plurals"
		))
		{
			qWarning(
				"Couldn't load translations."
			);
		}
	}
	installQtTranslator(
		language_
	);
	availableLanguages();
}

QList<QPair<QString, QString>> Translator::availableLanguages()
{
	const QStringList paths_ = {
#ifdef QT_DEBUG
	QString(
		"%1/share/translations"
	).arg(
		KEEPASSX_BINARY_DIR
	),
#endif
	FilePath::getInstance()->getDataPath(
		"translations"
	)
	};
	QList<QPair<QString, QString>> languages_;
	languages_.append(
		QPair<QString, QString>(
			"system",
			"System default"
		)
	);
	const QRegularExpression regExp_(
		"keepassx_([a-zA-Z_]+)\\.qm",
		QRegularExpression::CaseInsensitiveOption
	);
	for(const QString &path_: paths_)
	{
		const QStringList fileList_ = QDir(
			path_
		).entryList();
		for(const QString &filename_: fileList_)
		{
			if(QRegularExpressionMatch match_ = regExp_.match(
					filename_
				);
				match_.hasMatch())
			{
				QString langcode_ = match_.captured(
					1
				);
				if(langcode_ == "en_plurals")
				{
					langcode_ = "en";
				}
				QLocale locale_(
					langcode_
				);
				QString languageStr_ = QLocale::languageToString(
					locale_.language()
				);
				QString countryStr_;
				if(langcode_.contains(
					"_"
				))
				{
					countryStr_ = QString(
						" (%1)"
					).arg(
						QLocale::territoryToString(
							locale_.territory()
						)
					);
				}
				QPair<QString, QString> language_(
					langcode_,
					languageStr_ + countryStr_
				);
				languages_.append(
					language_
				);
			}
		}
	}
	return languages_;
}

bool Translator::installTranslator(
	const QString &language
)
{
	const QStringList paths_ = {
#ifdef QT_DEBUG
	QString(
		"%1/share/translations"
	).arg(
		KEEPASSX_BINARY_DIR
	),
#endif
	FilePath::getInstance()->getDataPath(
		"translations"
	)
	};
	for(const QString &path_: paths_)
	{
		if(installTranslator(
			language,
			path_
		))
		{
			return true;
		}
	}
	return false;
}

bool Translator::installTranslator(
	const QString &language,
	const QString &path
)
{
	const auto translator_ = new QTranslator(
		qApp
	);
	if(translator_->load(
		QString(
			"keepassx_"
		).append(
			language
		),
		path
	))
	{
		QCoreApplication::installTranslator(
			translator_
		);
		return true;
	}
	delete translator_;
	return false;
}

bool Translator::installQtTranslator(
	const QString &language
)
{
	const auto qtTranslator_ = new QTranslator(
		qApp
	);
	if(qtTranslator_->load(
		QString(
			"%1/qtbase_%2"
		).arg(
			QLibraryInfo::path(
				QLibraryInfo::TranslationsPath
			),
			language
		)
	))
	{
		QCoreApplication::installTranslator(
			qtTranslator_
		);
		return true;
	}
	delete qtTranslator_;
	return false;
}
