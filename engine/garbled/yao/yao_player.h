/*
 * garbled/yao/yao_player.h
 *
 */

#pragma once

#include "net/player.h"
#include "net/server.h"

class YaoPlayer
{
	string progname;
	Names N;

public:
	YaoPlayer(int argc, const char** argv);
	~YaoPlayer();
};
