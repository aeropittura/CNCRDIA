/*
 * boolean/ccd_prep.hpp
 *
 */

#pragma once

#include "boolean/ccd_prep.h"

#include "vm/processor.hpp"

namespace GC
{

template<class T>
CcdPrep<T>::~CcdPrep()
{
    if (part_processor)
        delete part_processor;
}

template<class T>
void CcdPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    auto& thread = ShareThread<T>::s();
    assert(thread.MC);

    if (part_processor)
    {
        assert(&part_processor->MC == &thread.MC->get_part_opener());
        assert(&part_processor->P == &protocol.get_part().P);
        return;
    }

    part_processor = new SubProcessor<typename T::part_type>(
            thread.MC->get_part_opener(), part_prep, protocol.get_part().P);
}

}
