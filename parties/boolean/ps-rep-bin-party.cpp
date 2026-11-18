/*
 * abfllnow-party.cpp
 *
 */

#include "boolean/postsacri_bin.h"
#include "boolean/postsacri_secret.h"
#include "boolean/rep_prep.h"

#include "boolean/share_party.hpp"
#include "boolean/rep_prep.hpp"
#include "protocols/replicated/malicious_rep_mc.hpp"

int main(int argc, const char** argv)
{
    GC::simple_binary_main<GC::PostSacriSecret>(argc, argv);
}
