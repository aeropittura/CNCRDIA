/*
 * garbled/bmr/trusted_party.h
 *
 */

#pragma once


#include "garbled/bmr/boolean_circuit.h"
#include "garbled/bmr/transport/node.h"
#include <atomic>

#include "garbled/bmr/register.h"
#include "garbled/bmr/common_party.h"

class BaseTrustedParty : virtual public CommonFakeParty {
public:
	vector<ReceivedMessage> prf_outputs;
	vector<SendBuffer> message_input_masks;

	BaseTrustedParty();
	virtual ~BaseTrustedParty();

	/* From NodeUpdatable class */
	virtual void node_ready();
	void NewMessage(int from, ReceivedMessage& msg);
	void node_aborted(struct sockaddr_in* from) { (void)from; }

	void Start();

protected:
	boost::mutex _print_mx;

	std::atomic_uint _num_prf_received;
	std::atomic_uint _received_gc_received;
	std::atomic_uint n_received;

	vector<SendBuffer> message_keys;

	int randomfd;

	bool done_filling;

	virtual bool _fill_keys() = 0;

	void _compute_send_garbled_circuit();
	virtual void _launch_online() = 0;

	void prepare_randomness();
	void send_randomness();
	virtual void send_input_masks(PartyId pid) = 0;
	virtual void send_output_masks() = 0;
	virtual void garble() = 0;

	void add_keys(const Register& reg);
};

class TrustedProgramParty : public BaseTrustedParty {
public:
    SendBuffer message_output_masks;

	TrustedProgramParty(int argc, char** argv);
	~TrustedProgramParty();

	void node_ready();

	void store_spdz_wire(SpdzOp op, const Register& reg);

	void store_wire(const Register& reg);
	void load_wire(Register& reg);

	const Key& delta(int i) { return deltas[i]; }
	const KeyVector& get_deltas() { return deltas; }

private:
	friend class GarbleRegister;
	friend class RandomRegister;

	static TrustedProgramParty* singleton;
	static TrustedProgramParty& s();

	GC::Machine< GC::Secret<GarbleRegister> > machine;
	GC::Processor< GC::Secret<GarbleRegister> > processor;
	GC::Program program;

	GC::Machine< GC::Secret<RandomRegister> > random_machine;
	GC::Processor< GC::Secret<RandomRegister> > random_processor;

	KeyVector deltas;

	vector<OctetStream> spdz_wires[SPDZ_OP_N];
	vector< Share<Gf2nLong> > mask_shares;

	Timer random_timer;

	bool _fill_keys();
	void _launch_online();

	void send_input_masks(PartyId pid);
	void send_output_masks();
	void garble();

	void add_all_keys(const Register& reg, bool external);
};

inline void BaseTrustedParty::add_keys(const Register& reg)
{
	for(int p = 0; p < get_n_parties(); p++)
		reg.keys.serialize(message_keys[p], p + 1);
}

inline void TrustedProgramParty::add_all_keys(const Register& reg, bool external)
{
	for(int p = 0; p < get_n_parties(); p++)
		for (int i = 0; i < get_n_parties(); i++)
			reg.keys[external][i].serialize(message_keys[p]);
}

inline TrustedProgramParty& TrustedProgramParty::s()
{
	if (singleton)
		return *singleton;
	else
		throw runtime_error("no singleton");
}
