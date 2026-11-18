/*
 * YaoPlayer.h
 *
 */

#ifndef YAO_YAOPLAYER_H_
#define YAO_YAOPLAYER_H_

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

#endif /* YAO_YAOPLAYER_H_ */
