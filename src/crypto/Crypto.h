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
#ifndef KEEPASSX_CRYPTO_H
#define KEEPASSX_CRYPTO_H
#include <QString>

class Crypto
{
public:
	static bool init();

	static bool getInitalized()
	{
		return initalized;
	};

	static bool backendSelfTest();

	static QString getErrorString()
	{
		return errorStr;
	};

	static QString getBackendVersion()
	{
		return QString(
			"libgcrypt "
		).append(
			backendVersion
		);
	};
private:
	Crypto();
	static bool checkAlgorithms();
	static bool selfTest();
	static void raiseError(
		const QString &str
	);
	static bool testSha256();
	static bool testAes256Cbc();
	static bool testAes256Ecb();
	static bool testTwofish();
	static bool testSalsa20();
	static bool initalized;
	static QString errorStr;
	static QString backendVersion;
};
#endif // KEEPASSX_CRYPTO_H
