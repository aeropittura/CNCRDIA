/*
 * ot/mascot_params.h
 *
 */

#pragma once

#include "support/offline_machine_base.h"

class MascotParameters : virtual public OfflineParameters
{
public:
    string prep_data_directory;
    bool generate_macs;
    bool amplify;
    bool check;
    bool correlation_check;
    bool generate_bits;
    bool use_extension;
    bool fewer_rounds;
    bool fiat_shamir;
    struct timeval start, stop;

    MascotParameters();

    void set_passive();
};
