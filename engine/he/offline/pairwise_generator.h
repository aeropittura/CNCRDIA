/*
 * he/offline/pairwise_generator.h
 *
 */

#pragma once

#include <vector>
using namespace std;

#include "he/offline/multiplier.h"
#include "he/offline/simple_generator.h"

class PairwiseMachine;

template <class FD>
class PairwiseGenerator : public GeneratorBase
{
    typedef typename FD::T T;

    friend MultiEncCommit<FD>;
    template<class U> friend class CowGearPrep;

    PlaintextVector<FD> a, b, c;
    AddableVector<RqElement> b_mod_q;
    vector<Multiplier<FD>*> multipliers;
    TripleProducer_<FD> producer;
    MultiEncCommit<FD> EC;
    MacCheck<T> MC;

    int n_ciphertexts;

    // temporary data
    AddableVector<Ciphertext> C;
    OctetStream ciphertexts, cleartexts;

    size_t volatile_memory;

    void run_with_errors();

public:
    PairwiseMachine& machine;

    vector<InputTuple<Share<typename FD::T>>> inputs;

    PairwiseGenerator(int thread_num, PairwiseMachine& machine, Player* player = 0);
    ~PairwiseGenerator();

    void run();
    void generate_inputs(int player);

    size_t report_size(ReportType type);
    void report_size(ReportType type, MemoryUsage& res);
    size_t report_sent();
};
