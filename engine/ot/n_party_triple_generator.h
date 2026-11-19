#pragma once

#include "net/player.h"
#include "ot/base_ot.h"
#include "support/random.h"
#include "support/time_func.h"
#include "vm/input_tuple.h"
#include "protocols/common/dabit.h"
#include "protocols/common/share.h"

#include "ot/ot_triple_setup.h"
#include "ot/mascot_params.h"
#include "ot/ot_multiplier.h"
#include "ot/mascot_mac_key.h"

#include <map>
#include <vector>

#define N_AMPLIFY 3

template <class T, int N>
class ShareTriple;
template <class T, int N>
class PlainTriple;

class GeneratorThread
{
protected:
    pthread_mutex_t mutex;
    pthread_cond_t ready;

public:
    int nTriples;

    map<string,Timer> timers;

    bool multi_threaded;

    GeneratorThread() : nTriples(0), multi_threaded(true) {}
    virtual ~GeneratorThread() {};
    virtual void generate() = 0;

    void lock();
    void unlock();
    void signal();
    void wait();
};

template<class T>
class OtTripleGenerator : public GeneratorThread
{
    typedef typename T::OpenType OpenType;
    typedef typename T::MacKeyType MacKeyType;
    typedef MascotMacKey<typename T::MacKeyType> CompleteMacKeyType;

    template<class> friend class OtTripleGenerator;

protected:
    static CompleteMacKeyType mac_key;

    //OtTripleSetup* setup;
    Player& global_player;
    Player* parent_player;

    int thread_num;
    int nbase;

    struct timeval last_lap;

    ofstream output_file;

    SeededPrng share_prg;

    void start_progress();
    void print_progress(int k);

    void signal_multipliers(MultJob job);
    void wait_for_multipliers();

    typename T::Multiplier* new_multiplier(int i);

public:
    // TwoPartyPlayer's for OTs, n-party Player for sacrificing
    vector<VirtualTwoPartyPlayer*> players;
    vector<typename T::Multiplier*> ot_multipliers;
    //vector<OTMachine*> machines;
    BitVector base_receiver_input; // same for every set of OTs
    vector< vector< array<BitVector, 2> > > base_sender_inputs;
    vector< vector<BitVector> > base_receiver_outputs;
    vector<BitVector> value_bits;
    BitVector b_padded_bits;

    int my_num;
    int n_triples_per_loop;
    int nloops;
    int field_size;
    int n_amplify;
    int n_preamp_triples_per_loop;
    int repeat[3];
    int nparties;

    MascotParameters& machine;

    vector<PlainTriple<OpenType, N_AMPLIFY>> preamp_triples;
    vector<array<OpenType, 3>> plain_triples;
    vector<Dabit<T>> plain_bits;
    vector<array<OpenType, 3>> mixedTriples;

    typename T::MacCheck* MC;

    static CompleteMacKeyType& get_mac_key() { return mac_key; }
    static MacKeyType get_mac_key(Player& P, bool = false);
    static void reset_mac_key();

    OtTripleGenerator(const OtTripleSetup& setup, const Names& names,
            int thread_num, int nTriples, int nloops, MascotParameters& machine,
            Player* parent_player = 0);
    ~OtTripleGenerator();

    void set_batch_size(int nTriples, bool limit = true);

    void generate() { throw NotImplemented(); }

    void generate_plain_triples();
    void plain_triple_round(int k = 0);

    void generate_plain_bits();
    void generate_mixed_triples();

    void run_multipliers(MultJob job);

    Player& get_player() { return global_player; }
};

template<class T>
class NPartyTripleGenerator : public OtTripleGenerator<T>
{
    typedef typename T::OpenType OpenType;
    typedef typename T::MacKeyType MacKeyType;

    virtual void generateTriples() { throw NotImplemented(); }
    virtual void generate_bits() { throw NotImplemented(); }

public:
    vector<InputTuple<typename T::InputType>> inputs;

    NPartyTripleGenerator(const OtTripleSetup& setup, const Names& names,
            int thread_num, int nTriples, int nloops, MascotParameters& machine,
            Player* parent_player = 0);
    virtual ~NPartyTripleGenerator() {}

    void generate();
    void generateInputs(int player);
};

template<class T>
typename OtTripleGenerator<T>::CompleteMacKeyType OtTripleGenerator<T>::mac_key;

template<class T>
class SimpleMascotTripleGenerator : public NPartyTripleGenerator<T>
{
    typedef typename T::MacKeyType MacKeyType;
    typedef typename T::MacCheck MacCheck;

    virtual void sacrifice(typename T::MacCheck&, PRNG&) { throw NotImplemented(); }

public:
    vector< ShareTriple<T, 2> > unchecked_triples;

    SimpleMascotTripleGenerator(const OtTripleSetup& setup, const Names& names,
            int thread_num, int nTriples, int nloops, MascotParameters& machine,
            Player* parent_player = 0);
    virtual ~SimpleMascotTripleGenerator() {}

    void generateTriples();
};

template<class T>
class MascotTripleGenerator : public SimpleMascotTripleGenerator<T>
{
    typedef typename T::OpenType OpenType;
    typedef typename T::MacKeyType MacKeyType;
    typedef typename T::MacCheck MacCheck;

    void generate_bits();
    void generate_bits_gf2n();
    template <int X, int L>
    void generate_bits_from_triples(MacCheck& MC, ofstream& output_file, Gfp_<X, L>);
    template <class U>
    void generate_bits_from_triples(MacCheck& MC, ofstream& output_file, U);

    void sacrifice(typename T::MacCheck& MC, PRNG& G);

public:
    vector<T> bits;

    MascotTripleGenerator(const OtTripleSetup& setup, const Names& names,
            int thread_num, int nTriples, int nloops, MascotParameters& machine,
            Player* parent_player = 0);
};

template<class T>
class Spdz2kTripleGenerator : public NPartyTripleGenerator<T>
{
    typedef typename T::OpenType OpenType;
    typedef typename T::MacKeyType MacKeyType;
    typedef typename T::SacriType SacriType;

    void generate_bits() { throw NotImplemented(); }

    template<class U>
    void sacrifice_z2k(U& MC, PRNG& G);

public:
    vector< ShareTriple<Share<SacriType>, 2> > unchecked_triples;

    Spdz2kTripleGenerator(const OtTripleSetup& setup, const Names& names,
            int thread_num, int nTriples, int nloops, MascotParameters& machine,
            Player* parent_player = 0);

    void generateTriples();
};
