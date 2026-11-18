/*
 * parties/ecdsa/Fake-ECDSA.cpp
 *
 */

#include "ecdsa/p256_element.h"
#include "support/mkpath.h"
#include "boolean/tinier_secret.h"

#include "protocols/fake/fake_stuff.hpp"
#include "protocols/common/share.hpp"
#include "vm/data_files.hpp"
#include "numeric/gfp.hpp"

int main()
{
    P256Element::init();
    KeySetup<Share<P256Element::Scalar>> key;
    string prefix = PREP_DIR "ECDSA/";
    mkdir_p(prefix.c_str());
    P256Element::Scalar::write_setup(prefix);
    PRNG G;
    G.ReSeed();
    generate_mac_keys<Share<P256Element::Scalar>>(key, 2, prefix, G);
    make_mult_triples<Share<P256Element::Scalar>>(key, 2, 1000, false, prefix, G);
    make_inverse<Share<P256Element::Scalar>>(key, 2, 1000, false, prefix, G);
    P256Element::finish();
}
