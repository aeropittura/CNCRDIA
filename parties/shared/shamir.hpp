/*
 * parties/shared/shamir.hpp
 *
 */

#pragma once

#include "protocols/shamir/shamir_options.h"
#include "protocols/shamir/shamir_share.h"
#include "protocols/shamir/malicious_shamir_share.h"
#include "numeric/gfp.h"
#include "numeric/gf2n.h"
#include "boolean/vector_protocol.h"
#include "boolean/ccd_prep.h"
#include "boolean/tiny_mc.h"
#include "boolean/malicious_ccd_secret.h"
#include "boolean/vector_input.h"

#include "vm/field_machine.hpp"

#include "vm/data_files.hpp"
#include "vm/instruction.hpp"
#include "vm/machine.hpp"
#include "protocols/shamir/shamir_input.hpp"
#include "protocols/shamir/shamir.hpp"
#include "protocols/shamir/shamir_mc.hpp"
#include "protocols/shamir/malicious_shamir_mc.hpp"
#include "protocols/shamir/malicious_shamir_po.hpp"
#include "protocols/common/mac_check_base.hpp"
#include "protocols/common/beaver.hpp"
#include "protocols/spdz2k/spdz2k_prep.hpp"
#include "protocols/common/replicated_prep.hpp"
#include "protocols/replicated/malrep_ring_prep.hpp"
#include "boolean/share_secret.hpp"
#include "boolean/vector_protocol.hpp"
#include "boolean/secret.hpp"
#include "boolean/ccd_prep.hpp"
#include "numeric/gfp.hpp"

template<template<class U> class T>
ShamirMachineSpec<T>::ShamirMachineSpec(int argc, const char** argv)
{
    auto& opts = ShamirOptions::singleton;
    ez::ezOptionParser opt;
    opts = {opt, argc, argv};
    HonestMajorityFieldMachine<T>(argc, argv, opt, opts.nparties);
}
