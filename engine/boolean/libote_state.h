/*
 * boolean/libote_state.h
 *
 */

#pragma once

#include "net/player_co_socket.h"

#include <secure-join/CorGenerator/CorGenerator.h>

namespace GC
{

template<class T>
class SemiSecureStateBase
{
    WaitQueue<const void*> job_queue, result_queue;

    pthread_t thread;

    static void* run_thread(void* state);

    void run();

public:
    macoro::thread_pool pools[2];
    macoro::thread_pool::work works[2];
    osuCrypto::PRNG prngs[2] = {osuCrypto::sysRandomSeed(), osuCrypto::sysRandomSeed()};
    secJoin::CorGenerator oles[2];

    array<PlayerCoSocket*, 4> co_sockets;
    Player& P;

    osuCrypto::BitVector base_bits;
    array<vector<array<PRNG, 2>>, 2> g_sender;
    array<vector<PRNG>, 2> g_receiver;

    SemiSecureStateBase(Player& P);
    ~SemiSecureStateBase();

    template<class U>
    void set_sender_base_ots(int role, U& sender);
    template<class U>
    void set_receiver_base_ots(int role, U& receiver);

    vector<osuCrypto::block> base_receiver_ots(int role, size_t size = 128);
    vector<array<osuCrypto::block, 2>> base_sender_ots(int role, size_t size =
            128);

    void generate(const void* job);
};

class SemiSecureShuffleState : public SemiSecureStateBase<SemiSecureShuffleState>
{
public:
    typedef const vector<SemiSecurePermutation*> ShuffleGeneratorType;
    typedef pair<size_t, ShuffleGeneratorType> JobType;

    void generate_role(JobType& job, int role);
};

class SemiSecureTripleState : public SemiSecureStateBase<SemiSecureTripleState>
{
public:
    typedef pair<int, array<vector<array<SemiSecret, 3>>, 2>> JobType;

    void generate_role(JobType& job, int role);
};

}
