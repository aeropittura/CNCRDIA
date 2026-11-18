/*
 * support/benchmarking.h
 *
 */

#pragma once

#include <stdexcept>
#include <string>
#include <iostream>
using namespace std;

// call before insecure benchmarking functionality
void insecure(string message, bool warning = true);

void insecure_fake(bool warning = true);
