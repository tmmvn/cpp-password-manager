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
#ifndef KEEPASSX_SYMMETRICCIPHERGCRYPT_H
#define KEEPASSX_SYMMETRICCIPHERGCRYPT_H
#include <gcrypt.h>
#include "crypto/SymmetricCipher.h"
#include "crypto/SymmetricCipherBackend.h"

class SymmetricCipherGcrypt final:public SymmetricCipherBackend
{
public:
	SymmetricCipherGcrypt(
		SymmetricCipher::Algorithm algo,
		SymmetricCipher::Mode mode,
		SymmetricCipher::Direction direction
	);
	virtual ~SymmetricCipherGcrypt() override;
	virtual bool init() override;
	virtual bool setKey(
		const QByteArray &key
	) override;
	virtual bool setIv(
		const QByteArray &iv
	) override;
	virtual QByteArray process(
		const QByteArray &data,
		bool* ok
	) override;
	Q_REQUIRED_RESULT virtual bool processInPlace(
		QByteArray &data
	) override;
	Q_REQUIRED_RESULT virtual bool processInPlace(
		QByteArray &data,
		quint64 rounds
	) override;
	virtual bool reset() override;
	virtual qint64 getBlockSize() const override;
	virtual QString getErrorString() const override;
private:
	static int gcryptAlgo(
		SymmetricCipher::Algorithm algo
	);
	static int gcryptMode(
		SymmetricCipher::Mode mode
	);
	void setErrorString(
		gcry_error_t err
	);
	gcry_cipher_hd_t ctx;
	const int algo;
	const int mode;
	const SymmetricCipher::Direction direction;
	QByteArray key;
	QByteArray iv;
	qint64 blockSize;
	QString errorString;
};
#endif // KEEPASSX_SYMMETRICCIPHERGCRYPT_H
