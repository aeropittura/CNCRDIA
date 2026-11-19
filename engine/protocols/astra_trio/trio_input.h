/*
 * protocols/astra_trio/trio_input.h
 *
 */

#pragma once

#include "protocols/astra_trio/astra_input.h"

template<class T>
class TrioInput : public AstraInput<T>
{
    typedef AstraInput<T> Super;

public:
    TrioInput(SubProcessor<T>& proc, MacCheckBase<T>& MC) :
            Super(proc, MC)
    {
    }
    TrioInput(SubProcessor<T>* proc, Player& P) :
            Super(proc, P)
    {
    }
    TrioInput(MacCheckBase<T>& MC, Preprocessing<T>& prep, Player& P,
            Trio<T>* protocol) :
            Super(MC, prep, P, protocol)
    {
    }

    T finalize_offset(int offset) final;
};
