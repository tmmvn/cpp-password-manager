/*
*  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
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
#ifndef KEEPASSX_LAYEREDSTREAM_H
#define KEEPASSX_LAYEREDSTREAM_H
#include <QIODevice>

class LayeredStream:public QIODevice
{
	Q_OBJECT public:
	explicit LayeredStream(
		QIODevice* baseDevice
	);
	virtual ~LayeredStream() override;
	virtual bool isSequential() const override;
	virtual bool open(
		OpenMode mode
	) override;
protected:
	virtual qint64 readData(
		char* data,
		qint64 maxSize
	) override;
	virtual qint64 writeData(
		const char* data,
		qint64 maxSize
	) override;

	QIODevice* getBaseDevice() const
	{
		return baseDevice;
	};
private:
	QIODevice* const baseDevice;
private Q_SLOTS:
	void do_CloseStream();
};
#endif // KEEPASSX_LAYEREDSTREAM_H
