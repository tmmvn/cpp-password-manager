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
#include "Random.h"
#include <gcrypt.h>
#include "core/Global.h"
#include "crypto/Crypto.h"

class RandomBackendGcrypt final:public RandomBackend
{
public:
	virtual void randomize(
		void* data,
		int len
	) override;
};

Random* Random::instance(
	nullptr
);

void Random::randomize(
	QByteArray &ba
) const
{
	this->backend->randomize(
		ba.data(),
		static_cast<int>(ba.size())
	);
}

QByteArray Random::getRandomArray(
	const int len
) const
{
	QByteArray ba_;
	ba_.resize(
		len
	);
	this->randomize(
		ba_
	);
	return ba_;
}

quint32 Random::getRandomUInt(
	const quint32 limit
) const
{
	if(limit == 0)
	{
		return 0;
	}
	if(limit == QUINT32_MAX)
	{
		return 0;
	}
	quint32 rand_;
	const quint32 ceil_ = QUINT32_MAX - (QUINT32_MAX % limit) - 1;
	// To avoid modulo bias:
	// Make sure rand is below the largest number where rand%limit==0
	do
	{
		this->backend->randomize(
			&rand_,
			4
		);
	}
	while(rand_ > ceil_);
	return (rand_ % limit);
}

quint32 Random::getRandomUIntRange(
	const quint32 min,
	const quint32 max
) const
{
	return min + getRandomUInt(
		max - min
	);
}

Random* Random::getInstance()
{
	if(!instance)
	{
		instance = new Random(
			new RandomBackendGcrypt()
		);
	}
	return instance;
}

void Random::createWithBackend(
	RandomBackend* backend
)
{
	if(backend == nullptr)
	{
		return;
	}
	if(instance != nullptr)
	{
		return;
	}
	instance = new Random(
		backend
	);
}

Random::Random(
	RandomBackend* backend
)
	: backend(
		backend
	)
{
}

void RandomBackendGcrypt::randomize(
	void* data,
	const int len
)
{
	if(!Crypto::getInitalized())
	{
		return;
	}
	gcry_randomize(
		data,
		len,
		GCRY_STRONG_RANDOM
	);
}
