/*
 * boolean/instruction_inline.h
 *
 */

#pragma once

#include "garbled/bmr/config.h"

#ifdef MAX_INLINE
#define MAYBE_INLINE inline
#else
#define MAYBE_INLINE
#endif

namespace GC {

#include "boolean/instructions.h"

template <class T>
inline bool fallback_code(const Instruction& instruction, Processor<T>& processor)
{
    (void)processor;
    cout << "Undefined instruction " << showbase << hex
            << instruction.get_opcode() << endl << dec;
    return true;
}

} /* namespace GC */
