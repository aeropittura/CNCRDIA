#pragma once

#include "net/player.h"
#include "ot/base_ot.h"
#include "support/random.h"
#include "support/time_func.h"

#include <map>

/*
 * Class for creating and storing base OTs between every pair of parties.
 */
class OtTripleSetup
{
    typedef vector< array<BitVector, 2> > SenderType;
    typedef vector<BitVector> ReceiverType;

    void run(int i);

    vector<BaseOT*> base_ots;

    PRNG G;
    int nparties;
    int my_num;

public:
    class SetupJob
    {
        OtTripleSetup& setup;
        int i;

    public:
        pthread_t thread;

        SetupJob(OtTripleSetup& setup, int i) :
                setup(setup), i(i), thread(0)
        {
        }

        void run()
        {
            setup.run(i);
        }
    };

    map<string,Timer> timers;
    vector<TwoPartyPlayer*> players;
    vector<SenderType> base_sender_inputs;
    vector<ReceiverType> base_receiver_outputs;
    BitVector base_receiver_inputs;

    int get_nparties() const { return nparties; }
    int get_nbase() const { return base_receiver_inputs.size(); }
    int get_my_num() const { return my_num; }
    int get_base_receiver_input(int i) const { return base_receiver_inputs[i]; }

    OtTripleSetup() : nparties(0), my_num(0)
    {
    }

    OtTripleSetup(Player& N, bool real_ots = true)
        : nparties(N.num_players()), my_num(N.my_num())
    {
        int nbase = 128;
        base_receiver_inputs.resize(nbase);
        base_ots.resize(nparties - 1);
        base_sender_inputs.resize(nparties - 1);
        base_receiver_outputs.resize(nparties - 1);

#ifdef VERBOSE_BASEOT
        if (real_ots)
            cout << "Doing real base OTs\n";
        else
            cout << "Doing fake base OTs\n";
#endif

        for (int i = 0; i < nparties - 1; i++)
        {
            int other_player;
            // i for indexing, other_player is actual number
            if (i >= my_num)
                other_player = i + 1;
            else
                other_player = i;

            players.push_back(new VirtualTwoPartyPlayer(N, other_player));

            // sets up a pair of base OTs, playing both roles
            if (real_ots)
            {
                base_ots[i] = new BaseOT(nbase, players[i]);
            }
            else
            {
                base_ots[i] = new FakeOt(nbase, players[i]);
            }
        }

        setup();
        close_connections();
    }

    // run the Base OTs
    void setup();
    // close down the sockets
    void close_connections();

    //template <class T>
    //T get_mac_key();

    OtTripleSetup get_fresh();

    BaseOT get_base(size_t index);

    int index_for(int other)
    {
        assert(other != my_num);
        if (other > my_num)
            return other - 1;
        else
            return other;
    }

    void pack(OctetStream& os) const;
    void unpack(OctetStream& os);

    void check(TwoPartyPlayer& P);
};

class OnDemandOtTripleSetup
{
    map<Player*, OtTripleSetup*> setups;

public:
    ~OnDemandOtTripleSetup()
    {
        for (auto& setup : setups)
            delete setup.second;
    }

    OtTripleSetup get_fresh(Player& P)
    {
        if (setups.find(&P) == setups.end())
            setups[&P] = new OtTripleSetup(P, true);
        return setups[&P]->get_fresh();
    }
};
