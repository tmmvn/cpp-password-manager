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
#include "CompositeKey.h"
#include <QtConcurrent>
#include "CompositeKey_p.h"
#include "core/Global.h"
#include "crypto/CryptoHash.h"
#include "crypto/SymmetricCipher.h"

CompositeKey::CompositeKey()
{
}

CompositeKey::CompositeKey(
	const CompositeKey &key
)
{
	*this = key;
}

CompositeKey::~CompositeKey()
{
	this->CompositeKey::clear();
}

void CompositeKey::clear()
{
	qDeleteAll(
		this->keys
	);
	this->keys.clear();
}

bool CompositeKey::isEmpty() const
{
	return this->keys.isEmpty();
}

CompositeKey* CompositeKey::clone() const
{
	return new CompositeKey(
		*this
	);
}

CompositeKey &CompositeKey::operator=(
	const CompositeKey &key
)
{
	// handle self assignment as that would break when calling clear()
	if(this == &key)
	{
		return *this;
	}
	this->clear();
	for(const Key* subKey_: asConst(
			key.keys
		))
	{
		this->addKey(
			*subKey_
		);
	}
	return *this;
}

QByteArray CompositeKey::rawKey() const
{
	CryptoHash cryptoHash_(
		CryptoHash::Sha256
	);
	for(const Key* key_: keys)
	{
		cryptoHash_.addData(
			key_->rawKey()
		);
	}
	return cryptoHash_.getResult();
}

QByteArray CompositeKey::transform(
	const QByteArray &seed,
	quint64 rounds,
	bool* ok,
	QString* errorString
) const
{
	// TODO: Should these return nullptr
	if(rounds == 0)
	{
		*ok = false;
		*errorString = "rounds must be > 0";
		return QByteArray();
	}
	if(seed.size() != 32)
	{
		*ok = false;
		*errorString = "seed must be 32 bytes";
		return QByteArray();
	}
	bool okLeft_;
	QString errorStringLeft_;
	bool okRight_;
	QString errorStringRight_;
	const QByteArray key_ = this->rawKey();
	const QFuture<QByteArray> future_ = QtConcurrent::run(
		this->transformKeyRaw,
		key_.left(
			16
		),
		seed,
		rounds,
		&okLeft_,
		&errorStringLeft_
	);
	const QByteArray result2_ = this->transformKeyRaw(
		key_.right(
			16
		),
		seed,
		rounds,
		&okRight_,
		&errorStringRight_
	);
	QByteArray transformed_;
	transformed_.append(
		future_.result()
	);
	transformed_.append(
		result2_
	);
	*ok = (okLeft_ && okRight_);
	if(!okLeft_)
	{
		*errorString = errorStringLeft_;
		return QByteArray();
	}
	if(!okRight_)
	{
		*errorString = errorStringRight_;
		return QByteArray();
	}
	return CryptoHash::hash(
		transformed_,
		CryptoHash::Sha256
	);
}

QByteArray CompositeKey::transformKeyRaw(
	const QByteArray &key,
	const QByteArray &seed,
	const quint64 rounds,
	bool* ok,
	QString* errorString
)
{
	const QByteArray iv_(
		16,
		0
	);
	SymmetricCipher cipher_(
		SymmetricCipher::Aes256,
		SymmetricCipher::Ecb,
		SymmetricCipher::Encrypt
	);
	if(!cipher_.init(
		seed,
		iv_
	))
	{
		*ok = false;
		*errorString = cipher_.getErrorString();
		return QByteArray();
	}
	QByteArray result_ = key;
	if(!cipher_.processInPlace(
		result_,
		rounds
	))
	{
		*ok = false;
		*errorString = cipher_.getErrorString();
		return QByteArray();
	}
	*ok = true;
	return result_;
}

void CompositeKey::addKey(
	const Key &key
)
{
	this->keys.append(
		key.clone()
	);
}

int CompositeKey::transformKeyBenchmark(
	const int msec
)
{
	TransformKeyBenchmarkThread thread1_(
		msec
	);
	TransformKeyBenchmarkThread thread2_(
		msec
	);
	thread1_.start();
	thread2_.start();
	thread1_.wait();
	thread2_.wait();
	return qMin(
		thread1_.getRounds(),
		thread2_.getRounds()
	);
}

TransformKeyBenchmarkThread::TransformKeyBenchmarkThread(
	const int msec
)
	: msec(
		msec
	),
	rounds(
		0
	)
{
	if(msec <= 0)
	{
		this->msec = 1;
	}
}

auto TransformKeyBenchmarkThread::getRounds() const
	-> int
{
	return this->rounds;
}

void TransformKeyBenchmarkThread::run()
{
	auto key_ = QByteArray(
		16,
		'\x7E'
	);
	const auto seed_ = QByteArray(
		32,
		'\x4B'
	);
	const QByteArray iv_(
		16,
		0
	);
	SymmetricCipher cipher_(
		SymmetricCipher::Aes256,
		SymmetricCipher::Ecb,
		SymmetricCipher::Encrypt
	);
	cipher_.init(
		seed_,
		iv_
	);
	QElapsedTimer t_;
	t_.start();
	do
	{
		if(!cipher_.processInPlace(
			key_,
			10000
		))
		{
			this->rounds = -1;
			return;
		}
		this->rounds += 10000;
	}
	while(!t_.hasExpired(
		this->msec
	));
}
