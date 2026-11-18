/*
 * minimal.hpp
 *
 */

// minimal header file to make all C++ code compile
// but not produce all templated code in binary
// use maximal.hpp for that

// please report if otherwise

#ifndef MACHINES_MINIMAL_HPP_
#define MACHINES_MINIMAL_HPP_

#include "parties/shared/h_files.h"

// some h files depend on hpp files

#include "boolean/secret.hpp"
#include "boolean/semi_secret.hpp"
#include "protocols/dealer/dealer_mc.hpp"
#include "protocols/common/mac_check.hpp"
#include "protocols/replicated/malicious_rep_mc.hpp"
#include "protocols/shamir/shamir_mc.hpp"

#endif /* MACHINES_MINIMAL_HPP_ */
