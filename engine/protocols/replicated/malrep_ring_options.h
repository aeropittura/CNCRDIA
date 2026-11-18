/*
 * MalRepRingOptions.h
 *
 */

#ifndef PROTOCOLS_MALREPRINGOPTIONS_H_
#define PROTOCOLS_MALREPRINGOPTIONS_H_

#include "support/ez_option_parser.h"

class MalRepRingOptions
{
public:
    static MalRepRingOptions singleton;

    bool shuffle;

    MalRepRingOptions();
    MalRepRingOptions(ez::ezOptionParser& opt, int argc, const char** argv);
};

#endif /* PROTOCOLS_MALREPRINGOPTIONS_H_ */
