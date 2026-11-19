/*
 * garbled/yao/yao_player.cpp
 *
 */

#include "garbled/yao/yao_player.h"
#include "garbled/yao/yao_garbler.h"
#include "garbled/yao/yao_evaluator.h"
#include "support/ez_option_parser.h"
#include "support/network_options.h"

#include "boolean/machine.hpp"

YaoPlayer::YaoPlayer(int argc, const char** argv)
{
	ez::EzOptionParser opt;
	opt.add(
			"", // Default.
			0, // Required?
			0, // Number of args expected.
			0, // Delimiter if expecting multiple args.
			"Evaluate only after garbling (very limited functionality).", // Help description.
			"-O", // Flag token.
			"--oneshot" // Flag token.
	);
	opt.add(
			"1024", // Default.
			0, // Required?
			1, // Number of args expected.
			0, // Delimiter if expecting multiple args.
			"Minimum number of gates for multithreading (default: 1024).", // Help description.
			"-t", // Flag token.
			"--threshold" // Flag token.
	);
	opt.add(
	        "100000", // Default.
	        0, // Required?
	        1, // Number of args expected.
	        0, // Delimiter if expecting multiple args.
	        "Size of circuit batches (default: 100000)", // Help description.
	        "-b", // Flag token.
	        "--batch-size" // Flag token.
	);
	auto& online_options = OnlineOptions::singleton;
	online_options = {opt, argc, argv, false};
	NetworkOptionsWithNumber network_options(opt, argc, argv, 2, false);
	online_options.finalize(opt, argc, argv);

	int my_num = online_options.playerno;
	int threshold;
	bool continuous = not opt.get("-O")->isSet;
	opt.get("-t")->getInt(threshold);
	opt.get("-b")->getInt(online_options.batch_size);
	progname = online_options.progname;

	GC::ThreadMasterBase* master;
	if (my_num == 0)
	    master = new YaoGarbleMaster(continuous, online_options, threshold);
	else
	    master = new YaoEvalMaster(continuous, online_options);

	network_options.start_networking(master->N, my_num);
	master->run(progname);

	if (my_num == 1)
	    ((YaoEvalMaster*)master)->machine.write_memory(0);

	delete master;
}

YaoPlayer::~YaoPlayer()
{
}
