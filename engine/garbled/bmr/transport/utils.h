/*
 * garbled/bmr/transport/utils.h
 *
 */

#pragma once


char cs(char* msg, unsigned int len, char result=0);

void phex (const void *addr, int len);
