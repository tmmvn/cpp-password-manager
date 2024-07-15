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
#ifndef KEEPASSX_CONFIG_H
#define KEEPASSX_CONFIG_H
#include <QVariant>
class QSettings;

class Config final:public QObject
{
	Q_OBJECT public:
	virtual ~Config() override;
	QVariant get(
		const QString &key
	) const;
	QVariant get(
		const QString &key,
		const QVariant &defaultValue
	) const;
	void set(
		const QString &key,
		const QVariant &value
	) const;
	static Config* getInstance();
	static void createConfigFromFile(
		const QString &file
	);
	static void createTempFileInstance();
private:
	Config(
		const QString &fileName,
		QObject* parent
	);
	explicit Config(
		QObject* parent
	);
	void init(
		const QString &fileName
	);
	static Config* instance;
	QScopedPointer<QSettings> settings;
	QHash<QString, QVariant> defaults;
	Q_DISABLE_COPY(
		Config
	)
};
#endif // KEEPASSX_CONFIG_H
