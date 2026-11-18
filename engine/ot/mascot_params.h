/*
 * ot/mascot_params.h
 *
 */

#pragma once

#include "support/offline_machine_base.h"

class MascotParams : virtual public OfflineParams
{
public:
    string prep_data_dir;
    bool generateMACs;
    bool amplify;
    bool check;
    bool correlation_check;
    bool generateBits;
    bool use_extension;
    bool fewer_rounds;
    bool fiat_shamir;
    struct timeval start, stop;

    MascotParams();

    void set_passive();
};
