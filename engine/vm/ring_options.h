/*
 * vm/ring_options.h
 *
 */

#pragma once

#include "support/ez_option_parser.h"
#include <string>
using namespace std;

class RingOptions
{
    bool R_is_set;

public:
    int R;

    RingOptions(ez::ezOptionParser& opt, int argc, const char** argv);

    int ring_size_from_opts_or_schedule(string progname);
};
