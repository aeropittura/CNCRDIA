/*
 * protocols/spdz/cowgear_options.h
 *
 */

#pragma once

#include "support/ez_option_parser.h"

class CowGearOptions
{
    bool use_top_gear;

public:
    static CowGearOptions singleton;

    int covert_security;

    CowGearOptions(bool covert = true);
    CowGearOptions(ez::EzOptionParser& opt, int argc, const char** argv,
            bool covert = true);

    bool top_gear()
    {
        return use_top_gear;
    }

    void set_top_gear(bool use)
    {
        use_top_gear = use;
    }
};
