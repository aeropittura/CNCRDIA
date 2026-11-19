/*
 * garbled/yao/yao_evaluator.h
 *
 */

#pragma once

#include "garbled/yao/yao_gate.h"
#include "garbled/yao/yao_eval_master.h"
#include "garbled/yao/yao_common.h"
#include "boolean/secret.h"
#include "boolean/thread.h"
#include "support/mmo.h"
#include "ot/ot_extension_with_matrix.h"

class YaoEvaluator: public GC::Thread<GC::Secret<YaoEvalWire>>,
		public YaoCommon<YaoEvalWire>
{
	typedef GC::Thread<GC::Secret<YaoEvalWire>> Super;

protected:
	static thread_local YaoEvaluator* singleton;

	ReceivedMessage gates;
	ReceivedMessageStore gates_store;

	YaoEvalMaster& master;

	friend class YaoCommon<YaoEvalWire>;
	friend class YaoEvalWire;

public:
	ReceivedMessage output_masks;
	ReceivedMessageStore output_masks_store;

	MMO mmo;

	RealTwoPartyPlayer player;
	OtExtensionWithMatrix ot_ext;

	static YaoEvaluator& s();

	YaoEvaluator(int thread_num, YaoEvalMaster& master);

	bool continuous() { return master.continuous; }

	void pre_run();
	void run(GC::Program& program);
	void run(GC::Program& program, Player& P);
	void run_from_store(GC::Program& program);
	bool receive(Player& P);
	void receive_to_store(Player& P);

	void load_gate(YaoGate& gate);

	long get_gate_id() { return gate_id(thread_num); }

	int get_n_worker_threads()
	{ return max(1u, thread::hardware_concurrency() / master.machine.nthreads); }

	NamedCommunicationStatistics extra_communication();
};

inline void YaoEvaluator::load_gate(YaoGate& gate)
{
	gates.unserialize(gate);
}

inline YaoEvaluator& YaoEvaluator::s()
{
	if (singleton)
		return *singleton;
	else
		throw runtime_error("singleton unavailable");
}
