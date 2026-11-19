/*
 * protocols/semi/hemi_options.h
 *
 */

#pragma once

#include "support/ez_option_parser.h"

class HemiOptions
{
public:
    static HemiOptions singleton;

    bool plain_matmul;


    HemiOptions() :
            plain_matmul(false)
    {
    }

    HemiOptions(ez::EzOptionParser& opt, int argc,
            const char** argv)
    {
        opt.add("", // Default.
                0, // Required?
                0, // Number of args expected.
                0, // Delimiter if expecting multiple args.
                "Don't use homomorphic matrix multplication", // Help description.
                "-M", // Flag token.
                "--plain-matmul" // Flag token.
                );

        opt.parse(argc, argv);
        plain_matmul = opt.isSet("-M");
        opt.reset_args();
    }
};
