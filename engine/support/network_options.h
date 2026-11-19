/*
 * support/network_options.h
 *
 */

#pragma once

#include "support/ez_option_parser.h"
#include "net/server.h"
#include "net/player.h"

#include <string>

class NetworkOptions
{
public:
    int portnum_base;
    std::string hostname;

    NetworkOptions(ez::EzOptionParser& opt, int argc, const char** argv);
};

class NetworkOptionsWithNumber : public NetworkOptions
{
public:
    int nplayers;
    std::string ip_filename;

    NetworkOptionsWithNumber(ez::EzOptionParser& opt, int argc,
            const char** argv, int default_nplayers, bool variable_nplayers);

    Server* start_networking(Names& N, int my_num);
};
