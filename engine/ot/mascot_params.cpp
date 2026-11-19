/*
 * ot/mascot_params.cpp
 *
 */

#include "ot/mascot_params.h"

MascotParameters::MascotParameters()
{
    generate_macs = true;
    amplify = true;
    check = true;
    correlation_check = true;
    generate_bits = false;
    use_extension = true;
    fewer_rounds = false;
    fiat_shamir = false;
    timerclear(&start);
}

void MascotParameters::set_passive()
{
    generate_macs = amplify = check = correlation_check = false;
}
