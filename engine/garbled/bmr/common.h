/*
 * garbled/bmr/common.h
 *
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <stdint.h>
using namespace std;

#include "support/check_vector.h"

typedef unsigned long WireId;
typedef unsigned long GateId;
typedef unsigned int PartyId;

class Function {
	bool rep[4];
	int shift(int i) { return 4 * (3 - i); }
public:
	Function() { memset(rep, 0, sizeof(rep)); }
	Function(std::string& func)
	{
		for (int i = 0; i < 4; i++)
			if (func[i] != '0')
				rep[i] = 1;
			else
				rep[i] = 0;
	}
	Function(int int_rep)
	{
		for (int i = 0; i < 4; i++)
			rep[i] = (int_rep << shift(i)) & 1;
	}
	uint8_t operator[](int i) { return rep[i]; }
	bool call(bool left, bool right) { return rep[2 * left + right]; }
};
