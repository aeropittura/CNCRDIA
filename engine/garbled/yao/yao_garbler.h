/*
 * garbled/yao/yao_garbler.h
 *
 */

#pragma once

#include "garbled/yao/yao_garble_wire.h"
#include "garbled/yao/yao_and_job.h"
#include "garbled/yao/yao_garble_master.h"
#include "garbled/yao/yao_common.h"
#include "support/random.h"
#include "support/mmo.h"
#include "boolean/secret.h"
#include "net/player.h"
#include "ot/ot_extension_with_matrix.h"

#include <thread>

class YaoGarbler: public GC::Thread<GC::Secret<YaoGarbleWire>>,
		public YaoCommon<YaoGarbleWire>
{
	friend class YaoGarbleWire;
	friend class YaoCommon<YaoGarbleWire>;

	typedef GC::Thread<GC::Secret<YaoGarbleWire>> Super;

protected:
	static thread_local YaoGarbler* singleton;

	YaoGarbleMaster& master;

	SendBuffer gates;

	Timer and_timer;
	Timer and_processor_timer;
	Timer and_main_thread_timer;
	DoubleTimer and_prepare_timer;
	DoubleTimer and_wait_timer;

public:
	PRNG prng;
	SendBuffer output_masks;
	MMO mmo;

	map<string, Timer> timers;

	RealTwoPartyPlayer player;
	OtExtensionWithMatrix ot_ext;

	deque<vector<Key>> receiver_input_keys;

	static YaoGarbler& s();

	YaoGarbler(int thread_num, YaoGarbleMaster& master);
	~YaoGarbler();

	bool continuous() { return master.continuous; }

	void run(GC::Program& program);
	void run(Player& P, bool continuous);
	void post_run();
	void send(Player& P);

	void process_receiver_inputs();

	Key get_delta() { return master.get_delta(); }
	void store_gate(const YaoGate& gate);

	int get_threshold() { return master.threshold; }

	long get_gate_id() { return gate_id(thread_num); }

    NamedCommunicationStatistics extra_communication();
};

inline YaoGarbler& YaoGarbler::s()
{
	if (singleton)
		return *singleton;
	else
		throw runtime_error("singleton unavailable");
}
