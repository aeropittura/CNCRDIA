/*
 * protocols/replicated/malrep_ring_options.h
 *
 */

#pragma once

#include "support/ez_option_parser.h"

class MalRepRingOptions
{
public:
    static MalRepRingOptions singleton;

    bool shuffle;

    MalRepRingOptions();
    MalRepRingOptions(ez::ezOptionParser& opt, int argc, const char** argv);
};
