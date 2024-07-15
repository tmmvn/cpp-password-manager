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
#include "Crypto.h"
#include <gcrypt.h>
#include <QMutex>
#include "crypto/CryptoHash.h"
#include "crypto/SymmetricCipher.h"
bool Crypto::initalized(
	false
);
QString Crypto::errorStr;
QString Crypto::backendVersion;

Crypto::Crypto()
{
}

bool Crypto::init()
{
	if(initalized)
	{
		qWarning(
			"Crypto::init: already initalized"
		);
		return true;
	}
	backendVersion = QString::fromLocal8Bit(
		gcry_check_version(
			nullptr
		)
	);
	gcry_control(
		GCRYCTL_INITIALIZATION_FINISHED,
		0
	);
	if(!checkAlgorithms())
	{
		return false;
	}
	// has to be set before testing Crypto classes
	initalized = true;
	if(!selfTest())
	{
		initalized = false;
		return false;
	}
	return true;
}

bool Crypto::backendSelfTest()
{
	return (gcry_control(
		GCRYCTL_SELFTEST
	) == 0);
}

bool Crypto::checkAlgorithms()
{
	if(gcry_cipher_algo_info(
		GCRY_CIPHER_AES256,
		GCRYCTL_TEST_ALGO,
		nullptr,
		nullptr
	) != 0)
	{
		errorStr = "GCRY_CIPHER_AES256 not found.";
		qWarning(
			"Crypto::checkAlgorithms: %s",
			qPrintable(
				errorStr
			)
		);
		return false;
	}
	if(gcry_cipher_algo_info(
		GCRY_CIPHER_TWOFISH,
		GCRYCTL_TEST_ALGO,
		nullptr,
		nullptr
	) != 0)
	{
		errorStr = "GCRY_CIPHER_TWOFISH not found.";
		qWarning(
			"Crypto::checkAlgorithms: %s",
			qPrintable(
				errorStr
			)
		);
		return false;
	}
	if(gcry_cipher_algo_info(
		GCRY_CIPHER_SALSA20,
		GCRYCTL_TEST_ALGO,
		nullptr,
		nullptr
	) != 0)
	{
		errorStr = "GCRY_CIPHER_SALSA20 not found.";
		qWarning(
			"Crypto::checkAlgorithms: %s",
			qPrintable(
				errorStr
			)
		);
		return false;
	}
	if(gcry_md_test_algo(
		GCRY_MD_SHA256
	) != 0)
	{
		errorStr = "GCRY_MD_SHA256 not found.";
		qWarning(
			"Crypto::checkAlgorithms: %s",
			qPrintable(
				errorStr
			)
		);
		return false;
	}
	return true;
}

bool Crypto::selfTest()
{
	return testSha256() && testAes256Cbc() && testAes256Ecb() && testTwofish()
		&& testSalsa20();
}

void Crypto::raiseError(
	const QString &str
)
{
	errorStr = str;
	qWarning(
		"Crypto::selfTest: %s",
		qPrintable(
			errorStr
		)
	);
}

bool Crypto::testSha256()
{
	if(const QByteArray sha256Test_ = CryptoHash::hash(
			"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
			CryptoHash::Sha256
		);
		sha256Test_ != QByteArray::fromHex(
			"248D6A61D20638B8E5C026930C3E6039A33CE45964FF2167F6ECEDD419DB06C1"
		))
	{
		raiseError(
			"SHA-256 mismatch."
		);
		return false;
	}
	return true;
}

bool Crypto::testAes256Cbc()
{
	const QByteArray key_ = QByteArray::fromHex(
		"603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4"
	);
	const QByteArray iv_ = QByteArray::fromHex(
		"000102030405060708090a0b0c0d0e0f"
	);
	QByteArray plainText_ = QByteArray::fromHex(
		"6bc1bee22e409f96e93d7e117393172a"
	);
	plainText_.append(
		QByteArray::fromHex(
			"ae2d8a571e03ac9c9eb76fac45af8e51"
		)
	);
	QByteArray cipherText_ = QByteArray::fromHex(
		"f58c4c04d6e5f1ba779eabfb5f7bfbd6"
	);
	cipherText_.append(
		QByteArray::fromHex(
			"9cfc4e967edb808d679f777bc6702c7d"
		)
	);
	bool ok_;
	SymmetricCipher aes256Encrypt_(
		SymmetricCipher::Aes256,
		SymmetricCipher::Cbc,
		SymmetricCipher::Encrypt
	);
	if(!aes256Encrypt_.init(
		key_,
		iv_
	))
	{
		raiseError(
			aes256Encrypt_.getErrorString()
		);
		return false;
	}
	const QByteArray encryptedText_ = aes256Encrypt_.process(
		plainText_,
		&ok_
	);
	if(!ok_)
	{
		raiseError(
			aes256Encrypt_.getErrorString()
		);
		return false;
	}
	if(encryptedText_ != cipherText_)
	{
		raiseError(
			"AES-256 CBC encryption mismatch."
		);
		return false;
	}
	SymmetricCipher aes256Decrypt_(
		SymmetricCipher::Aes256,
		SymmetricCipher::Cbc,
		SymmetricCipher::Decrypt
	);
	if(!aes256Decrypt_.init(
		key_,
		iv_
	))
	{
		raiseError(
			aes256Decrypt_.getErrorString()
		);
		return false;
	}
	const QByteArray decryptedText_ = aes256Decrypt_.process(
		cipherText_,
		&ok_
	);
	if(!ok_)
	{
		raiseError(
			aes256Decrypt_.getErrorString()
		);
		return false;
	}
	if(decryptedText_ != plainText_)
	{
		raiseError(
			"AES-256 CBC decryption mismatch."
		);
		return false;
	}
	return true;
}

bool Crypto::testAes256Ecb()
{
	const QByteArray key_ = QByteArray::fromHex(
		"000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"
	);
	const QByteArray iv_ = QByteArray::fromHex(
		"00000000000000000000000000000000"
	);
	QByteArray plainText_ = QByteArray::fromHex(
		"00112233445566778899AABBCCDDEEFF"
	);
	plainText_.append(
		QByteArray::fromHex(
			"00112233445566778899AABBCCDDEEFF"
		)
	);
	QByteArray cipherText_ = QByteArray::fromHex(
		"8EA2B7CA516745BFEAFC49904B496089"
	);
	cipherText_.append(
		QByteArray::fromHex(
			"8EA2B7CA516745BFEAFC49904B496089"
		)
	);
	bool ok_;
	SymmetricCipher aes256Encrypt_(
		SymmetricCipher::Aes256,
		SymmetricCipher::Ecb,
		SymmetricCipher::Encrypt
	);
	if(!aes256Encrypt_.init(
		key_,
		iv_
	))
	{
		raiseError(
			aes256Encrypt_.getErrorString()
		);
		return false;
	}
	const QByteArray encryptedText_ = aes256Encrypt_.process(
		plainText_,
		&ok_
	);
	if(!ok_)
	{
		raiseError(
			aes256Encrypt_.getErrorString()
		);
		return false;
	}
	if(encryptedText_ != cipherText_)
	{
		raiseError(
			"AES-256 ECB encryption mismatch."
		);
		return false;
	}
	SymmetricCipher aes256Decrypt_(
		SymmetricCipher::Aes256,
		SymmetricCipher::Ecb,
		SymmetricCipher::Decrypt
	);
	if(!aes256Decrypt_.init(
		key_,
		iv_
	))
	{
		raiseError(
			aes256Decrypt_.getErrorString()
		);
		return false;
	}
	const QByteArray decryptedText_ = aes256Decrypt_.process(
		cipherText_,
		&ok_
	);
	if(!ok_)
	{
		raiseError(
			aes256Decrypt_.getErrorString()
		);
		return false;
	}
	if(decryptedText_ != plainText_)
	{
		raiseError(
			"AES-256 ECB decryption mismatch."
		);
		return false;
	}
	return true;
}

bool Crypto::testTwofish()
{
	const QByteArray key_ = QByteArray::fromHex(
		"603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4"
	);
	const QByteArray iv_ = QByteArray::fromHex(
		"000102030405060708090a0b0c0d0e0f"
	);
	QByteArray plainText_ = QByteArray::fromHex(
		"6bc1bee22e409f96e93d7e117393172a"
	);
	plainText_.append(
		QByteArray::fromHex(
			"ae2d8a571e03ac9c9eb76fac45af8e51"
		)
	);
	QByteArray cipherText_ = QByteArray::fromHex(
		"e0227c3cc80f3cb1b2ed847cc6f57d3c"
	);
	cipherText_.append(
		QByteArray::fromHex(
			"657b1e7960b30fb7c8d62e72ae37c3a0"
		)
	);
	bool ok_;
	SymmetricCipher twofishEncrypt_(
		SymmetricCipher::Twofish,
		SymmetricCipher::Cbc,
		SymmetricCipher::Encrypt
	);
	if(!twofishEncrypt_.init(
		key_,
		iv_
	))
	{
		raiseError(
			twofishEncrypt_.getErrorString()
		);
		return false;
	}
	const QByteArray encryptedText_ = twofishEncrypt_.process(
		plainText_,
		&ok_
	);
	if(!ok_)
	{
		raiseError(
			twofishEncrypt_.getErrorString()
		);
		return false;
	}
	if(encryptedText_ != cipherText_)
	{
		raiseError(
			"Twofish encryption mismatch."
		);
		return false;
	}
	SymmetricCipher twofishDecrypt_(
		SymmetricCipher::Twofish,
		SymmetricCipher::Cbc,
		SymmetricCipher::Decrypt
	);
	if(!twofishDecrypt_.init(
		key_,
		iv_
	))
	{
		raiseError(
			twofishEncrypt_.getErrorString()
		);
		return false;
	}
	const QByteArray decryptedText_ = twofishDecrypt_.process(
		cipherText_,
		&ok_
	);
	if(!ok_)
	{
		raiseError(
			twofishDecrypt_.getErrorString()
		);
		return false;
	}
	if(decryptedText_ != plainText_)
	{
		raiseError(
			"Twofish encryption mismatch."
		);
		return false;
	}
	return true;
}

bool Crypto::testSalsa20()
{
	const QByteArray salsa20Key_ = QByteArray::fromHex(
		"F3F4F5F6F7F8F9FAFBFCFDFEFF000102030405060708090A0B0C0D0E0F101112"
	);
	const QByteArray salsa20iv_ = QByteArray::fromHex(
		"0000000000000000"
	);
	const QByteArray salsa20Plain_ = QByteArray::fromHex(
		"00000000000000000000000000000000"
	);
	const QByteArray salsa20Cipher_ = QByteArray::fromHex(
		"B4C0AFA503BE7FC29A62058166D56F8F"
	);
	bool ok_;
	SymmetricCipher salsa20Stream_(
		SymmetricCipher::Salsa20,
		SymmetricCipher::Stream,
		SymmetricCipher::Encrypt
	);
	if(!salsa20Stream_.init(
		salsa20Key_,
		salsa20iv_
	))
	{
		raiseError(
			salsa20Stream_.getErrorString()
		);
		return false;
	}
	const QByteArray salsaProcessed_ = salsa20Stream_.process(
		salsa20Plain_,
		&ok_
	);
	if(!ok_)
	{
		raiseError(
			salsa20Stream_.getErrorString()
		);
		return false;
	}
	if(salsaProcessed_ != salsa20Cipher_)
	{
		raiseError(
			"Salsa20 stream cipher mismatch."
		);
		return false;
	}
	return true;
}
