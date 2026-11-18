/*
 * garbled/yao/config.h
 *
 */

#pragma once

//#define DEBUG

//#define CHECK_BUFFER

class YaoFullGate;
class YaoHalfGate;

#ifndef FULL_GATES
typedef YaoHalfGate YaoGate;
#else
typedef YaoFullGate YaoGate;
#endif
