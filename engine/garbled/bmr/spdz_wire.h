/*
 * garbled/bmr/spdz_wire.h
 *
 */

#pragma once

#include "protocols/common/share.h"
#include "garbled/bmr/key.h"

template<class T>
class DualWire
{
public:
	T mask;
	Key my_keys[2];

	DualWire()
	{
		my_keys[0] = 0;
		my_keys[1] = 0;
	}

	void pack(octetStream& os) const
	{
		mask.pack(os);
		os.serialize(my_keys);
	}
	void unpack(octetStream& os, size_t wanted_size)
	{
		(void)wanted_size;
		mask.unpack(os);
		os.unserialize(my_keys);
	}
};

typedef DualWire<Share<gf2n_long>> SpdzWire;
