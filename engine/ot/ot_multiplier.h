/*
 * ot/ot_multiplier.h
 *
 */

#pragma once

#include <vector>
using namespace std;

#include "ot/ot_extension_with_matrix.h"
#include "ot/ot_vole.h"
#include "ot/rectangle.h"
#include "support/random.h"
#include "support/check_vector.h"

template<class T>
class NPartyTripleGenerator;
template<class T>
class OtTripleGenerator;

class MultJob
{
public:
    Dtype type;
    bool input;
    int player;
    int n_inputs;
    MultJob(Dtype type = N_DTYPE) : type(type), input(false), player(-1), n_inputs(0) {}
    MultJob(int player, int n_inputs) : type(N_DTYPE), input(true), player(player), n_inputs(n_inputs) {}
};

class OtMultiplierBase
{
public:
    pthread_t thread;
    WaitQueue<MultJob> inbox;
    WaitQueue<MultJob> outbox;

    template<class U>
    static void init(U& key, Player& P, int other, OtTripleSetup& ot_setup);

    virtual ~OtMultiplierBase() {}
    virtual void multiply() = 0;
    virtual void multiply_raw() = 0;
};

template <class V, class W>
class OtMultiplierMac : public OtMultiplierBase
{
public:
    vector< vector<V> > macs;
    vector<W> input_macs;
};

template <class T>
class OtMultiplier : public OtMultiplierMac<typename T::SacriType, typename T::MacType>
{
    void multiply_raw();

protected:
    BitVector key_bits;
    vector< array<BitVector, 2> > senderOutput;
    vector<BitVector> receiverOutput;

    virtual void multiply_for_triples();
    virtual void multiply_for_bits();
    virtual void multiply_for_mixed();
	virtual void multiply_for_inputs(MultJob job) = 0;

    virtual void after_correlation() = 0;
    virtual void initialize_authenticator(const BitVector& base_receiver_input,
            const vector< array<BitVector, 2> >& base_sender_input,
            const vector<BitVector>& base_receiver_output) = 0;

public:
    OtTripleGenerator<T>& generator;
    int thread_num;
    OtExtensionWithMatrix rot_ext;

    OtCorrelator<Matrix<typename T::Rectangle> > ot_correlator;

    OtMultiplier(OtTripleGenerator<T>& generator, int thread_num);
    virtual ~OtMultiplier();

    void init();
    void multiply();
};

template <class T>
class MascotMultiplier : public OtMultiplier<T>
{
    OtCorrelator<Matrix<typename T::Square> > auth_ot_ext;
    void after_correlation();
    void initialize_authenticator(const BitVector& base_receiver_input,
            const vector< array<BitVector, 2> >& base_sender_input,
            const vector<BitVector>& base_receiver_output);

    void multiply_for_bits();
    template <int = 0>
    void multiply_for_bits(true_type);
    template <int = 0>
    void multiply_for_bits(false_type);

public:
    vector<typename T::OpenType> c_output;

    MascotMultiplier(OtTripleGenerator<T>& generator, int thread_num);

	void multiply_for_inputs(MultJob job);
};

template <class T>
class TinyMultiplier : public OtMultiplier<T>
{
    OtVole<typename T::part_type::SacriType> mac_vole;

    void after_correlation();
    void initialize_authenticator(const BitVector& base_receiver_input,
            const vector< array<BitVector, 2> >& base_sender_input,
            const vector<BitVector>& base_receiver_output);

public:
    vector<typename T::OpenType> c_output;

    TinyMultiplier(OtTripleGenerator<T>& generator, int thread_num);

    void multiply_for_inputs(MultJob job) { (void) job; throw NotImplemented(); }
};

template <class T>
class TinierMultiplier : public OtMultiplier<T>
{
    OtExtensionWithMatrix auth_ot_ext;

    void after_correlation();
    void initialize_authenticator(const BitVector& base_receiver_input,
            const vector< array<BitVector, 2> >& base_sender_input,
            const vector<BitVector>& base_receiver_output);

public:
    vector<typename T::OpenType> c_output;

    TinierMultiplier(OtTripleGenerator<T>& generator, int thread_num);

    void multiply_for_inputs(MultJob job);
};

template <int K, int S> class Spdz2kShare;

template <int K, int S>
class Spdz2kMultiplier: public OtMultiplier<Spdz2kShare<K, S>>
{
    typedef Spdz2kShare<K, S> T;

    void after_correlation();
    void initialize_authenticator(const BitVector& base_receiver_input,
            const vector< array<BitVector, 2> >& base_sender_input,
            const vector<BitVector>& base_receiver_output);

    void multiply_for_inputs(MultJob job);

public:
    static const int TAU = TAU(K, S);
    static const int PASSIVE_MULT_BITS = K + S;
    static const int MAC_BITS = K + 2 * S;

    vector<Z2kRectangle<TAU, PASSIVE_MULT_BITS> > c_output;
    OtVoleBase<Z2<MAC_BITS>>* mac_vole;
    OtVoleBase<Z2<K + S>>* input_mac_vole;

    Spdz2kMultiplier(OtTripleGenerator<T>& generator, int thread_num);
    ~Spdz2kMultiplier();
};

template<class T>
class SemiMultiplier : public OtMultiplier<T>
{
    void multiply_for_inputs(MultJob job)
    {
        (void) job;
        throw NotImplemented();
    }

    void multiply_for_triples(false_type);
    void multiply_for_triples(true_type);

    void multiply_for_triples();
    void multiply_for_bits();
    void multiply_for_mixed();

    void after_correlation();

    void initialize_authenticator(const BitVector& base_receiver_input,
            const vector< array<BitVector, 2> >& base_sender_input,
            const vector<BitVector>& base_receiver_output)
    {
        (void) base_receiver_input, (void) base_receiver_output, (void) base_sender_input;
    }

public:
    CheckVector<typename T::OpenType> c_output;

    SemiMultiplier(OtTripleGenerator<T>& generator, int i) :
            OtMultiplier<T>(generator, i)
    {
    }
};
