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
#ifndef KEEPASSX_SYMMETRICCIPHER_H
#define KEEPASSX_SYMMETRICCIPHER_H
#include <QByteArray>
#include <QScopedPointer>
#include <QString>
#include "crypto/SymmetricCipherBackend.h"

class SymmetricCipher
{
public:
	enum Algorithm: uint8_t
	{
		Aes256,
		Twofish,
		Salsa20
	};

	enum Mode: uint8_t
	{
		Cbc,
		Ecb,
		Stream
	};

	enum Direction: uint8_t
	{
		Decrypt,
		Encrypt
	};

	SymmetricCipher(
		Algorithm algo,
		Mode mode,
		Direction direction
	);
	~SymmetricCipher();
	bool init(
		const QByteArray &key,
		const QByteArray &iv
	);
	bool isInitalized() const;

	QByteArray process(
		const QByteArray &data,
		bool* ok
	) const
	{
		return backend->process(
			data,
			ok
		);
	}

	Q_REQUIRED_RESULT bool processInPlace(
		QByteArray &data
	) const
	{
		return backend->processInPlace(
			data
		);
	}

	Q_REQUIRED_RESULT bool processInPlace(
		QByteArray &data,
		const quint64 rounds
	) const
	{
		if(rounds == 0)
		{
			return false;
		}
		return backend->processInPlace(
			data,
			rounds
		);
	}

	bool reset() const;
	qint64 getBlockSize() const;
	QString getErrorString() const;
private:
	const QScopedPointer<SymmetricCipherBackend> backend;
	bool initialized;
	Q_DISABLE_COPY(
		SymmetricCipher
	)
};
#endif // KEEPASSX_SYMMETRICCIPHER_H
