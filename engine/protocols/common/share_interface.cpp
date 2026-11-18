/*
 * ShareInterface.cpp
 *
 */

#include "protocols/common/share_interface.h"
#include "boolean/no_share.h"

const int ShareInterface::default_length;

const false_type ShareInterface::triple_matmul;
const false_type ShareInterface::is_clear;

GC::NoValue ShareInterface::get_mac_key()
{
    throw runtime_error("no MAC");
}

void ShareInterface::set_mac_key(GC::NoValue)
{
}
