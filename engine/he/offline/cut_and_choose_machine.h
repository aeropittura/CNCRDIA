/*
 * he/offline/cut_and_choose_machine.h
 *
 */

#pragma once

#include "he/offline/simple_machine.h"

class CutAndChooseMachine : public MultiplicativeMachine
{
    bool covert;

    template <class FD>
    GeneratorBase* new_generator(int i);

public:
    CutAndChooseMachine(int argc, const char** argv);

    int get_covert() const { return sec; }
};
