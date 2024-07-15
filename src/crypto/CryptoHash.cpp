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
#include "CryptoHash.h"
#include <gcrypt.h>
#include "crypto/Crypto.h"

class CryptoHashPrivate
{
public:
	gcry_md_hd_t ctx;
	unsigned int hashLen;
};

CryptoHash::CryptoHash(
	const Algorithm algo
)
	: d_ptr(
		new CryptoHashPrivate()
	)
{
	Q_D(
		CryptoHash
	);
	if(!Crypto::getInitalized())
	{
		return;
	}
	int algoGcrypt_ = GCRY_MD_SHA256;
	switch(algo)
	{
		case Sha256:
			algoGcrypt_ = GCRY_MD_SHA256;
			break;
		default:
			break;
	}
	const gcry_error_t error_ = gcry_md_open(
		&d->ctx,
		algoGcrypt_,
		0
	);
	if(error_ != 0)
	{
		return;
	}
	d->hashLen = gcry_md_get_algo_dlen(
		algoGcrypt_
	);
}

CryptoHash::~CryptoHash()
{
	Q_D(
		CryptoHash
	);
	gcry_md_close(
		d->ctx
	);
	delete this->d_ptr;
}

void CryptoHash::addData(
	const QByteArray &data
)
{
	Q_D(
		CryptoHash
	);
	if(data.isEmpty())
	{
		return;
	}
	gcry_md_write(
		d->ctx,
		data.constData(),
		data.size()
	);
}

void CryptoHash::reset()
{
	Q_D(
		CryptoHash
	);
	gcry_md_reset(
		d->ctx
	);
}

QByteArray CryptoHash::getResult() const
{
	Q_D(
		const CryptoHash
	);
	const auto result_ = reinterpret_cast<const char*>(gcry_md_read(
		d->ctx,
		0
	));
	return QByteArray(
		result_,
		d->hashLen
	);
}

QByteArray CryptoHash::hash(
	const QByteArray &data,
	const Algorithm algo
)
{
	// replace with gcry_md_hash_buffer()?
	CryptoHash cryptoHash_(
		algo
	);
	cryptoHash_.addData(
		data
	);
	return cryptoHash_.getResult();
}
