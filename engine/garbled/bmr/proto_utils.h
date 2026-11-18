/*
 * garbled/bmr/proto_utils.h
 *
 */

#pragma once

#include "garbled/bmr/msg_types.h"
#include <time.h>
#include <sys/time.h>
#include <vector>
#include <iostream>
using namespace std;

#include "support/avx_memcpy.h"
#include "support/flex_buffer.h"

#define LOOPBACK_STR "LOOPBACK"

class SendBuffer;

void fill_message_type(void* buffer, MSG_TYPE type);
void fill_message_type(SendBuffer& buffer, MSG_TYPE type);

void phex (const void *addr, int len);


inline void phex(const FlexBuffer& buffer) { phex(buffer.data(), buffer.size()); }
