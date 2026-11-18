/*
 * garbled/yao/yao_garble_input.h
 *
 */

#pragma once

#include "garbled/yao/yao_garbler.h"

class YaoGarbleWire;

class YaoGarbleInput : public InputterBase
{
public:
    YaoGarbler& garbler;

    YaoGarbleInput() :
            garbler(YaoGarbler::s())
    {
    }

    void exchange()
    {
        garbler.receiver_input_keys.push_back({});
    }
};
