/*
 * garbled/bmr/garbled_gate.cpp
 *
 */

#include "garbled/bmr/garbled_gate.h"
#include "garbled/bmr/prf.h"
#include "garbled/bmr/common_party.h"

GarbledGate::~GarbledGate() {
}

void GarbledGate::initialize_inputs(GateId g, int n_parties)
{
	n_parties = CommonParty::get_n_parties();
	id = g;
	for (unsigned int e = 0; e <= 1; e++) {
		prf_inputs[e].resize(n_parties);
		for (unsigned int j = 0; j < (size_t)n_parties; j++) {
			prf_inputs[e][j] = 0;

			/* fill out this buffer s.t. first 4 bytes are the extension (0/1),
			 * next 4 bytes are gate_id and next 4 bytes are party id.
			 * For the first half we dont need to fill the extension because
			 * it is zero anyway.
			 */

			unsigned int* prf_input_index =
					(unsigned int*) &prf_inputs[e][j]; //easier to refer as integers
			//				printf("e,g,j=%u,%u,%u\n",e,g,j);
			*prf_input_index = e;
			*(prf_input_index + 1) = g;
			*(prf_input_index + 2) = j + 1;
		}
	}
}

void GarbledGate::compute_prfs_outputs(const Register** in_wires, int my_id,
        PrfOutputs& prf_output, GateId g)
{
    int n_parties = CommonParty::get_n_parties();
    initialize_inputs(g, n_parties);
    for(int w=0; w<=1; w++) {
        for (int b=0; b<=1; b++) {
            const Key& key = in_wires[w]->key(my_id, b);
            __m128i rd_key[15];
            aes_128_schedule((Octet*) rd_key, (unsigned char*)&key.r);
#ifdef DEBUG
            cout << "using key " << key << endl;
#endif
            for (int e=0; e<=1; e++) {
                for (int j=1; j<= n_parties; j++) {
                    prf_output[j-1].outputs[w][b][e][0] =
                            aes_128_encrypt(*(__m128i*)input(e, j), (Octet*)rd_key);
                }
            }
        }
    }
}

void GarbledGate::compute_prfs_outputs(const Register** in_wires, int my_id,
        SendBuffer& buffer, GateId g)
{
    int n_parties = CommonParty::get_n_parties();
    PrfOutputs prf_output(n_parties);
    compute_prfs_outputs(in_wires, my_id, prf_output, g);
    prf_output.serialize(buffer, my_id, n_parties);
#ifdef DEBUG
	WireId wire_ids[] = { (WireId)in_wires[0]->get_id(), (WireId)in_wires[1]->get_id() };
	prf_output.print_prfs(g, wire_ids, my_id, n_parties);
#endif
}

void PrfOutputs::serialize(SendBuffer& buffer, int my_id, int n_parties)
{
    (void) my_id;
    for (int i = 0; i < n_parties; i++)
        buffer.serialize(tuples[i]);
}

void PrfOutputs::print_prfs(GateId g, WireId* in_wires, PartyId my_id, int n_parties)
{
	for(int w=0; w<=1; w++) {
		for (int b=0; b<=1; b++) {
			for (int e=0; e<=1; e++) {
				for(PartyId j=1; j<=(size_t)n_parties; j++) {
					printf("F_k^%d_{%lu,%u}(%d,%lu,%u) = ", my_id, in_wires[w], b, e, g, j);
					Key k = *((Key*)(*this)[j-1].outputs[w][b][e]);
					std::cout << k << std::endl;
				}
			}
		}
	}
}

void GarbledGate::print()
{
	cout << "garbled gate " << id << endl;
	for (int i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < keys[i].size(); j++)
			cout << keys[i][j] << " ";
		cout << endl;
	}
}
