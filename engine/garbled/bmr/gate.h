
#pragma once

#include <string>
#include <iostream>
#include <stdio.h>

#include "garbled/bmr/key.h"
#include "garbled/bmr/common.h"

#define NO_LAYER (-1)

class Gate {
public:
	WireId _left;
	WireId _right;
	WireId _out;
	Function _func;

	int _layer;

	Gate() : _left(-1), _right(-1), _out(-1), _layer(-1) {}

	inline void init(WireId left, WireId right, WireId out,std::string func) {
		_left = left;
		_right = right;
		_out = out;
		_func = func;
		_layer = NO_LAYER;
	}
	inline uint8_t func(uint8_t left, uint8_t right) {
		return _func[2*left+right];
	}

	void print(int id) {
		printf ("gate %d: l:%lu, r:%lu, o:%lu, func:%d%d%d%d\n", id, _left, _right, _out, _func[0], _func[1], _func[2], _func[3] );
	}

};
