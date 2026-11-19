/*
 * garbled/bmr/party.h
 *
 */

#pragma once

#include "garbled/bmr/register.h"
#include "garbled/bmr/garbled_gate.h"
#include "garbled/bmr/transport/node.h"
#include "garbled/bmr/common_party.h"
#include "garbled/bmr/spdz_wire.h"
#include "garbled/bmr/and_job.h"

#include "boolean/machine.h"
#include "boolean/program.h"
#include "boolean/processor.h"
#include "boolean/secret.h"
#include "boolean/runtime_branching.h"
#include "support/worker.h"

#define SERVER_ID (0)
#define INPUT_KEYS_MSG_TYPE_SIZE (16) // so memory will by aligned

#ifndef N_EVAL_THREADS
// Default Intel desktop processor has 8 half cores.
// This is beneficial if only one AES available per full core.
#define N_EVAL_THREADS (thread::hardware_concurrency())
#endif


class PartyProperties
{
protected:
    PartyId _id;

	Timer online_timer;

	Key delta;

public:
	PartyProperties() : _id(-1) {}

	PartyId get_id() { return _id; }
	Key get_delta() { return delta; }

};

class BaseParty : virtual public CommonFakeParty, virtual public PartyProperties
{
public:
    BaseParty();
    virtual ~BaseParty();

	/* From NodeUpdatable class */
	void node_ready();
	void NewMessage(int from, ReceivedMessage& msg);
	void node_aborted(struct sockaddr_in* from) { (void)from; }

	void Start();

protected:
	virtual void _compute_prfs_outputs(Key* keys) = 0;
	void _send_prfs();

	virtual void store_garbled_circuit(ReceivedMessage& msg) = 0;
	virtual void _check_evaluate() = 0;

	virtual void mask_output(ReceivedMessage& msg) = 0;
	virtual void mask_input(ReceivedMessage& msg) = 0;

	void done();

	virtual void start_online_round() = 0;

	virtual void receive_spdz_wires(ReceivedMessage& msg) = 0;
};

class ProgramParty : virtual public CommonParty, virtual public PartyProperties, public GC::RuntimeBranching
{
protected:
	friend class PrfRegister;
	friend class EvalRegister;
	friend class Register;

	vector<char> prf_output;

	deque<OctetStream> spdz_wires[SPDZ_OP_N];
	size_t spdz_storage;
	size_t garbled_storage;
	vector<size_t> spdz_counters;

	Worker<AndJob>* eval_threads;
	vector<AndJob> and_jobs;

	ReceivedMessageStore output_masks_store;
	ReceivedMessageStore input_masks_store;

	GC::Machine< GC::Secret<EvalRegister> > machine;
	GC::Processor<GC::Secret<EvalRegister> > processor;
	GC::Program program;

	GC::Machine< GC::Secret<PrfRegister> > prf_machine;
	GC::Processor<GC::Secret<PrfRegister> > prf_processor;

	void store_garbled_circuit(ReceivedMessage& msg);
	void load_garbled_circuit();

	virtual void _check_evaluate() = 0;
	virtual void done() = 0;

	virtual void receive_keys(Register& reg) = 0;
	virtual void receive_all_keys(Register& reg, bool external) = 0;
	virtual void process_prf_output(PrfOutputs& prf_output,
			PrfRegister* out, const PrfRegister* left, const PrfRegister* right) = 0;

	void start_online_round();

	void mask_output(ReceivedMessage& msg) { output_masks_store.push(msg); }
	void mask_input(ReceivedMessage& msg) { input_masks_store.push(msg); }

public:
	static ProgramParty* singleton;

	LocalBuffer garbled_circuit;
	ReceivedMessageStore garbled_circuits;

	LocalBuffer output_masks;
	LocalBuffer input_masks;

	Player* P;
	Names N;

	int threshold;

	vector<Integer> convcbit;

	static ProgramParty& s();

	ProgramParty();
	virtual ~ProgramParty();

	void reset();

	void store_wire(const Register& reg);
	void load_wire(Register& reg);
};

template<class T>
class ProgramPartySpec : public ProgramParty
{
	static ProgramPartySpec* singleton;

protected:
	GC::Memory<T> dynamic_memory;

	void _check_evaluate();

public:
	typename T::MacCheck* MC;

	static ProgramPartySpec& s();

	ProgramPartySpec();
	~ProgramPartySpec();

	void load(string progname);

	void get_spdz_wire(SpdzOp op, DualWire<T>& spdz_wire);
};

typedef ProgramPartySpec<Share<Gf2nLong>> FakeProgramPartySuper;

class FakeProgramParty : virtual public BaseParty, virtual public FakeProgramPartySuper
{
    Key* keys_for_prf;

	void _compute_prfs_outputs(Key* keys);

	void store_garbled_circuit(ReceivedMessage& msg) {  ProgramParty::store_garbled_circuit(msg); }

	void _check_evaluate();

	void receive_keys(Register& reg);
	void receive_all_keys(Register& reg, bool external);
	void process_prf_output(PrfOutputs& prf_output, PrfRegister* out,
			const PrfRegister* left, const PrfRegister* right);

	void receive_spdz_wires(ReceivedMessage& msg);

	void start_online_round() { FakeProgramPartySuper::start_online_round(); }

	void mask_output(ReceivedMessage& msg) { ProgramParty::mask_output(msg); }
	void mask_input(ReceivedMessage& msg) { ProgramParty::mask_input(msg); }

	void done() { BaseParty::done(); }

public:
    FakeProgramParty(int argc, const char** argv);
    ~FakeProgramParty();
};

inline ProgramParty& ProgramParty::s()
{
	if (singleton)
		return *singleton;
	else
		throw runtime_error("no singleton");
}

template<class T>
inline ProgramPartySpec<T>& ProgramPartySpec<T>::s()
{
	if (singleton)
		return *singleton;
	else
		throw runtime_error("no singleton");
}
