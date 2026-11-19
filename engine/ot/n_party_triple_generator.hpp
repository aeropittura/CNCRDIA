#pragma once

#include "ot/n_party_triple_generator.h"

#include "ot/ot_extension_with_matrix.h"
#include "ot/ot_multiplier.h"
#include "support/subroutines.h"
#include "protocols/common/mac_check.h"
#include "boolean/semi_secret.h"
#include "boolean/semi_prep.h"
#include "vm/online_options.h"

#include "ot/triple.hpp"
#include "ot/ot_multiplier.hpp"
#include "ot/mascot_mac_key.hpp"
#include "protocols/common/mac_check.hpp"
#include "protocols/semi/semi_input.hpp"
#include "protocols/semi/semi_mc.hpp"
#include "protocols/common/mac_key.hpp"

#include <sstream>
#include <fstream>
#include <math.h>

template<class T>
void* run_ot_thread(void* ptr)
{
    Bigint::initialize_thread();
    auto multiplier = (OtMultiplierBase*) ptr;
    if (OnlineOptions::singleton.has_option("throw_exceptions"))
        multiplier->multiply();
    else
    {
        try
        {
            multiplier->multiply();
        }
        catch (exception& e)
        {
            cerr << "Fatal error in OT thread: " << e.what() << endl;
            exit(1);
        }
    }
    return NULL;
}

template<class T>
typename OtTripleGenerator<T>::MacKeyType OtTripleGenerator<T>::get_mac_key(
        Player& P, bool)
{
    if (mac_key == 0)
    {
        mac_key.template read_or_generate<T>(P);
        OtTripleGenerator<typename T::BitPrepType>::mac_key = mac_key.get_fresh();
    }
    return mac_key;
}

template<class T>
void OtTripleGenerator<T>::reset_mac_key()
{
	mac_key = {};
}

/*
 * Copies the relevant base OTs from setup
 * N.B. setup must not be stored as it will be used by other threads
 */
template<class T>
NPartyTripleGenerator<T>::NPartyTripleGenerator(const OtTripleSetup& setup,
        const Names& names, int thread_num, int _n_triples, int nloops,
        MascotParameters& machine, Player* parent_player) :
        OtTripleGenerator<T>(setup, names, thread_num, _n_triples, nloops,
                machine, parent_player)
{
}

template<class T>
SimpleMascotTripleGenerator<T>::SimpleMascotTripleGenerator(const OtTripleSetup& setup,
        const Names& names, int thread_num, int _n_triples, int nloops,
        MascotParameters& machine, Player* parent_player) :
        NPartyTripleGenerator<T>(setup, names, thread_num, _n_triples, nloops,
                machine, parent_player)
{
}

template<class T>
MascotTripleGenerator<T>::MascotTripleGenerator(const OtTripleSetup& setup,
        const Names& names, int thread_num, int _n_triples, int nloops,
        MascotParameters& machine, Player* parent_player) :
        SimpleMascotTripleGenerator<T>(setup, names, thread_num, _n_triples, nloops,
                machine, parent_player)
{
}

template<class T>
Spdz2kTripleGenerator<T>::Spdz2kTripleGenerator(const OtTripleSetup& setup,
        const Names& names, int thread_num, int _n_triples, int nloops,
        MascotParameters& machine, Player* parent_player) :
        NPartyTripleGenerator<T>(setup, names, thread_num, _n_triples, nloops,
                machine, parent_player)
{
}

template<class T>
void OtTripleGenerator<T>::set_batch_size(int batch_size, bool limit)
{
    // limit to ~1 GB
    if (limit)
        batch_size = min(batch_size, int(1e7 / sizeof(T) / sizeof(T)));
    if (OnlineOptions::singleton.has_option("verbose_ot"))
        fprintf(stderr, "OT batch size %d (share size %d)\n", batch_size,
                int(sizeof(T)));
    n_triples_per_loop = DIV_CEIL(batch_size, nloops);
    nTriples = n_triples_per_loop * nloops;
    n_preamp_triples_per_loop = n_triples_per_loop * n_amplify;
}

template<class T>
OtTripleGenerator<T>::OtTripleGenerator(const OtTripleSetup& setup,
        const Names& names, int thread_num, int _n_triples, int nloops,
        MascotParameters& machine, Player* parent_player) :
        global_player(parent_player ? *parent_player : *new PlainPlayer(names,
                to_string(thread_num))),
        parent_player(parent_player),
        thread_num(thread_num),
        my_num(setup.get_my_num()),
        nloops(nloops),
        nparties(setup.get_nparties()),
        machine(machine),
        MC(0)
{
    field_size = T::OpenType::size() * 8;
    n_amplify = machine.amplify ? N_AMPLIFY : 1;
    set_batch_size(_n_triples);

    int n = nparties;
    //base_receiver_input = machines[0]->base_receiver_input;
    //base_sender_inputs.resize(n-1);
    //base_receiver_outputs.resize(n-1);
    nbase = setup.get_nbase();
    base_receiver_input.resize(nbase);
    base_receiver_outputs = setup.base_receiver_outputs;
    base_sender_inputs = setup.base_sender_inputs;
    assert(size_t(nbase) == base_sender_inputs.at(0).size());
    players.resize(n-1);

    // copy base OT inputs + outputs
    for (int j = 0; j < 128; j++)
    {
        base_receiver_input.set_bit(j, (unsigned int)setup.get_base_receiver_input(j));
    }

    for (int i = 0; i < n-1; i++)
    {
        // i for indexing, other_player is actual number
        int other_player;
        if (i >= my_num)
            other_player = i + 1;
        else
            other_player = i;

        players[i] = new VirtualTwoPartyPlayer(global_player, other_player);
    }

    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&ready, 0);

    ot_multipliers.resize(nparties-1);

    for (int i = 0; i < nparties-1; i++)
    {
        ot_multipliers[i] = new_multiplier(i);
        pthread_create(&(ot_multipliers[i]->thread), 0, run_ot_thread<T>, ot_multipliers[i]);
    }

    wait_for_multipliers();
}

template<class T>
OtTripleGenerator<T>::~OtTripleGenerator()
{
    // wait for threads to finish
    for (int i = 0; i < nparties-1; i++)
    {
        ot_multipliers[i]->inbox.stop();
        pthread_join(ot_multipliers[i]->thread, NULL);
#ifdef DEBUG_THREADS
        cout << "OT thread " << i << " finished\n" << flush;
#endif
    }

    for (size_t i = 0; i < ot_multipliers.size(); i++)
        delete ot_multipliers[i];

    for (size_t i = 0; i < players.size(); i++)
        delete players[i];
    //delete nplayer;
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&ready);

    if (parent_player != &global_player)
        delete &global_player;
}

template<class T>
typename T::Multiplier* OtTripleGenerator<T>::new_multiplier(int i)
{
    return new typename T::Multiplier(*this, i);
}

template<class T>
void NPartyTripleGenerator<T>::generate()
{
    Bigint::initialize_thread();

    auto& timers = this->timers;
    auto& machine = this->machine;
    auto& my_num = this->my_num;
    auto& thread_num = this->thread_num;
    auto& nTriples = this->nTriples;
    auto& output_file = this->output_file;

    timers["Generator thread"].start();

    // add up the shares from each thread and write to file
    stringstream ss;
    ss << machine.prep_data_directory;
    if (machine.generate_bits)
        ss << "Bits-";
    else
        ss << "Triples-";
    ss << T::type_short() << "-P" << my_num;
    if (thread_num != 0)
        ss << "-" << thread_num;
    if (machine.output)
    {
        output_file.open(ss.str().c_str());
        if (machine.generate_macs or not T::clear::invertible)
            file_signature<T>(this->mac_key).output(output_file);
        else
            file_signature<SemiShare<typename T::clear>>().output(output_file);
    }

    if (machine.generate_bits)
        generate_bits();
    else
        generateTriples();

    timers["Generator thread"].stop();
    if (machine.output)
        cout << "Written " << nTriples << " " << T::type_string() << " outputs to " << ss.str() << endl;
#ifdef VERBOSE_OT
    else
        cerr << "Generated " << nTriples << " " << T::type_string() << " outputs" << endl;
#endif
}

template<class W>
void NPartyTripleGenerator<W>::generateInputs(int player)
{
    CODE_LOCATION
    typedef typename W::InputType::ShareType::OpenType T;

    auto n_triples_per_loop = this->n_triples_per_loop;
    auto& value_bits = this->value_bits;
    auto& share_prg = this->share_prg;
    auto& ot_multipliers = this->ot_multipliers;
    auto& nparties = this->nparties;
    auto& global_player = this->global_player;

    if (this->thread_num >= 0)
        n_triples_per_loop *= 10;

    // extra value for sacrifice
    int toCheck = n_triples_per_loop
            + DIV_CEIL(W::MacKeyType::size_in_bits(), T::size_in_bits());

    if (OnlineOptions::singleton.has_option("verbose_input"))
        fprintf(stderr, "generating %d input tuples\n", toCheck);

    value_bits.resize(1);
    this->signal_multipliers({player, toCheck});
    bool mine = player == global_player.my_num();

    if (mine)
    {
        value_bits[0].resize(toCheck * T::size_in_bits());
        value_bits[0].template randomize_blocks<T>(share_prg);
        this->signal_multipliers({});
    }

    this->wait_for_multipliers();

    GlobalPrng G(global_player);
    typename W::InputCheckType check_sum;
    inputs.resize(toCheck);
    typename W::MacType mac_key = this->get_mac_key();
    SemiInput<SemiShare<T>> input(0, global_player);
    input.maybe_initialize(global_player);
    input.reset_all(global_player);
    vector<T> secrets(toCheck);
    if (mine)
        for (int j = 0; j < toCheck; j++)
        {
            secrets[j] = value_bits[0].template get_portion<T>(j);
            input.add_mine(secrets[j]);
        }
    input.exchange();
    bool debug = OnlineOptions::singleton.has_option("debug_mac");
    if (debug)
        cerr << "MAC key: " << mac_key << endl;

    for (int j = 0; j < toCheck; j++)
    {
        T share;
        typename W::MacType mac_sum;
        share = input.finalize(player);
        if (mine)
        {
            mac_sum = secrets[j] * mac_key;
            for (int i = 0; i < nparties-1; i++)
                mac_sum += (ot_multipliers[i])->input_macs[j];
        }
        else
        {
            int i_thread = player - (player > global_player.my_num() ? 1 : 0);
            mac_sum = (ot_multipliers[i_thread])->input_macs[j];
        }
        inputs[j] = {{share, mac_sum}, secrets[j]};
        auto r = G.get<typename W::InputCheckType::ShareType>();
        check_sum += typename W::InputCheckType(r * share, r * mac_sum);

        if (debug)
            cerr << "share=" << inputs[j].share << " value=" << inputs[j].value
                    << endl;
    }
    inputs.resize(n_triples_per_loop);

    if (debug)
        cerr << "check=" << check_sum << endl;

    typename W::InputCheckType::MacCheck MC(mac_key);
    // use zero element because all is perfectly randomized
    MC.set_random_element({});
    MC.POpen(check_sum, global_player);
    MC.Check(global_player);
}

template<class T>
void MascotTripleGenerator<T>::generate_bits_gf2n()
{
    CODE_LOCATION
    auto& bits = this->bits;
    auto& global_player = this->global_player;
    auto& n_triples_per_loop = this->n_triples_per_loop;
    auto& value_bits = this->value_bits;

    this->signal_multipliers(DATA_BIT);

    int n_bits_to_check = this->n_triples_per_loop + this->field_size;
    value_bits.resize(1);
    value_bits[0].resize(ceil(1.0 * n_bits_to_check / 128) * 128);
    bits.resize(n_bits_to_check);
    vector<T> to_open(1);
    vector<typename T::clear> opened(1);
    MacCheck_<T> MC(this->get_mac_key());

    this->start_progress();

    for (int k = 0; k < this->nloops; k++)
    {
        this->print_progress(k);

        value_bits[0].randomize(this->share_prg);

        this->signal_multipliers({});
        this->timers["Authentication OTs"].start();
        this->wait_for_multipliers();
        this->timers["Authentication OTs"].stop();

        Octet seed[SEED_SIZE];
        create_random_seed(seed, this->global_player, SEED_SIZE);
        PRNG G;
        G.SetSeed(seed);

        T check_sum;
        typename T::clear r;
        for (int j = 0; j < n_bits_to_check; j++)
        {
            auto mac_sum =
                    value_bits[0].get_bit(j) ?
                            typename T::clear(this->get_mac_key()) : 0;
            for (int i = 0; i < this->nparties-1; i++)
                mac_sum += this->ot_multipliers[i]->macs[0][j];
            bits[j].set_share(value_bits[0].get_bit(j));
            bits[j].set_mac(mac_sum);
            r.randomize(G);
            check_sum += r * bits[j];
        }
        bits.resize(n_triples_per_loop);

        to_open[0] = check_sum;
        MC.POpen_Begin(opened, to_open, global_player);
        MC.POpen_End(opened, to_open, global_player);
        MC.Check(global_player);

        if (this->machine.output)
            for (int j = 0; j < n_triples_per_loop; j++)
                bits[j].output(this->output_file, false);
   }
}

template<class T>
void MascotTripleGenerator<T>::generate_bits()
{
    if (T::clear::characteristic_two)
        generate_bits_gf2n();
    else
        this->generateTriples();
}

template<class T>
void Spdz2kTripleGenerator<T>::generateTriples()
{
    CODE_LOCATION
	const int K = T::k;
	const int S = T::s;
	auto& unchecked_triples = this->unchecked_triples;

	auto& timers = this->timers;
	auto& n_triples_per_loop = this->n_triples_per_loop;
	auto& value_bits = this->value_bits;
	auto& share_prg = this->share_prg;
	auto& ot_multipliers = this->ot_multipliers;
	auto& nparties = this->nparties;
	auto& global_player = this->global_player;
	auto& nloops = this->nloops;
	auto& b_padded_bits = this->b_padded_bits;

	this->signal_multipliers(DATA_TRIPLE);

	const int TAU = Spdz2kMultiplier<K, S>::TAU;
	const int TAU_ROUNDED = (TAU + 7) / 8 * 8;
	value_bits.resize(3);
	for (int i = 0; i < 2; i++)
		value_bits[2*i].resize(max(2 * 8 * Z2<K + 2 * S>::N_BYTES, TAU_ROUNDED) * n_triples_per_loop);
	value_bits[1].resize(8 * Z2<K + S>::N_BYTES * (n_triples_per_loop + 1));
	b_padded_bits.resize(8 * Z2<K + 2 * S>::N_BYTES * (n_triples_per_loop + 1));
	vector< PlainTriple_<Z2<K + 2 * S>, Z2<K + S>, 2> > amplified_triples(n_triples_per_loop);
	unchecked_triples.resize(n_triples_per_loop);
	typename T::PrepCheckType::MacCheck MC(this->get_mac_key());

	this->start_progress();

	for (int k = 0; k < nloops; k++)
	{
		this->print_progress(k);

		for (int j = 0; j < 2; j++)
			value_bits[j].randomize(share_prg);

		for (int j = 0; j < n_triples_per_loop + 1; j++)
		{
			Z2<K + S> b(value_bits[1].get_pointer_to_byte(j, Z2<K + S>::N_BYTES));
			b_padded_bits.set_portion(j, Z2<K + 2 * S>(b));
		}

		timers["OTs"].start();
		this->signal_multipliers({});
		this->wait_for_multipliers();
		timers["OTs"].stop();

		Octet seed[SEED_SIZE];
		create_random_seed(seed, global_player, SEED_SIZE);
		PRNG G;
		G.SetSeed(seed);

		BitVector aBits = value_bits[0];

		for (int j = 0; j < n_triples_per_loop; j++)
		{
			BitVector a(aBits.get_pointer_to_bit(j, TAU_ROUNDED), TAU);
			Z2<K + S> b(value_bits[1].get_pointer_to_byte(j, Z2<K + S>::N_BYTES));
			Z2kRectangle<TAU, K + S> c;
			c.mul(a, b);
			timers["Triple computation"].start();
			for (int i = 0; i < nparties-1; i++)
			{
				c += ot_multipliers[i]->c_output[j];
			}

#ifdef DEBUG_SPDZ2K
			for (int l = 0; l < c.N_ROWS; l++)
			{
				Z2<K + S> z = fake.POpen({c.rows[l], {}}, global_player);
				auto x = fake.POpen({a.get_bit(l), {}}, global_player);
				auto y = fake.POpen({b, {}}, global_player);
				Z2<K + S> zz = x * y;
				if (z != zz)
				{
					cout << dec << j << " " << l << " " << hex << z << " " << zz
							<< " " << y << " " << x << " " << a.get_byte(l / 8) << endl;
				}
			}
#endif

			timers["Triple computation"].stop();
			amplified_triples[j].amplify(a, b, c, G);
			amplified_triples[j].to(value_bits, j);
		}

		this->signal_multipliers({});
		this->wait_for_multipliers();

		for (int j = 0; j < n_triples_per_loop; j++)
		{
			unchecked_triples[j].from(amplified_triples[j], j, *this);
		}

		// we can skip the consistency check since we're doing a mac-check next
		// get piggy-backed random value
		Z2<K + 2 * S> r_share = b_padded_bits.get_pointer_to_byte(n_triples_per_loop, Z2<K + 2 * S>::N_BYTES);
		Z2<K + 2 * S> r_mac;
		r_mac = (r_share * this->get_mac_key());
		for (int i = 0; i < this->nparties-1; i++)
			r_mac += (ot_multipliers[i])->macs.at(1).at(n_triples_per_loop);
		Share<Z2<K + 2 * S>> r;
		r.set_share(r_share);
		r.set_mac(r_mac);

		MC.set_random_element(r);
		sacrifice_z2k(MC, G);
	}
}

template<class U>
void OtTripleGenerator<U>::generate_plain_triples()
{
    machine.set_passive();
    machine.output = false;
    signal_multipliers(DATA_TRIPLE);

    value_bits.resize(3);
    for (int i = 0; i < 3; i++)
        value_bits[i].resize(field_size * n_preamp_triples_per_loop);

    start_progress();
    for (int i = 0; i < nloops; i++)
        plain_triple_round(i);
}

template<class T>
void OtTripleGenerator<T>::generate_plain_bits()
{
    CODE_LOCATION
    assert(ot_multipliers.size() == 1);

    machine.set_passive();
    machine.output = false;

    int batch_size = n_preamp_triples_per_loop;
    int n = multiple_minimum(batch_size, T::Rectangle::n_rows_allocated());

    if (OnlineOptions::singleton.has_option("debug_plain"))
        cerr << "Generating " << n << " plain bits" << endl;

    value_bits.resize(1);
    value_bits[0].resize(n);
    value_bits[0].randomize(share_prg);

    signal_multipliers(DATA_BIT);

    wait_for_multipliers();
    plain_bits.clear();

    typename T::OpenType two = 2;

    for (int j = 0; j < n; j++)
    {
        if (j % T::Rectangle::n_rows_allocated() < T::OpenType::length())
        {
            bool b = value_bits[0].get_bit(j);
            plain_bits.push_back({b, b});
            plain_bits.back().first += ot_multipliers[0]->c_output[j] * two;
        }
    }
}

template<class T>
void OtTripleGenerator<T>::generate_mixed_triples()
{
    CODE_LOCATION
    assert(ot_multipliers.size() == 1);

    machine.set_passive();
    machine.output = false;

    int n = multiple_minimum(n_preamp_triples_per_loop, 8);

    if (OnlineOptions::singleton.has_option("verbose_mixed"))
        fprintf(stderr, "generating %d mixed triples\n", n);

    value_bits.resize(2);
    value_bits[0].resize(n);
    value_bits[0].randomize(share_prg);
    value_bits[1].resize(n * T::OpenType::N_BITS);
    value_bits[1].randomize(share_prg);

    signal_multipliers(DATA_MIXED);

    wait_for_multipliers();
    mixedTriples.clear();

    for (int j = 0; j < n; j++)
    {
        auto a = value_bits[0].get_bit(j);
        auto b = value_bits[1].template get_portion<typename T::OpenType>(j);
        auto c = a ? b : typename T::OpenType();
        for (auto& x : ot_multipliers)
            c += x->c_output[j];
        mixedTriples.push_back({{a, b, c}});
    }
}

template<class U>
void OtTripleGenerator<U>::plain_triple_round(int k)
{
    CODE_LOCATION
    if (OnlineOptions::singleton.has_option("verbose_triples"))
        fprintf(stderr, "generating %d triples\n", n_preamp_triples_per_loop);

    typedef typename U::OpenType T;

    if (not (machine.amplify or machine.output))
        plain_triples.resize(n_preamp_triples_per_loop);

    print_progress(k);

    for (int j = 0; j < 2; j++)
        value_bits[j].template randomize_blocks<T>(share_prg);

    timers["OTs"].start();
    for (int i = 0; i < nparties-1; i++)
        ot_multipliers[i]->inbox.push({});
    this->wait_for_multipliers();
    timers["OTs"].stop();

    for (int j = 0; j < n_preamp_triples_per_loop; j++)
    {
        T a;
        a.assign((char*)value_bits[0].get_ptr() + j * T::size());
        T b;
        b.assign((char*)value_bits[1].get_ptr() + j / n_amplify * T::size());
        T c = a * b;
        timers["Triple computation"].start();
        for (int i = 0; i < nparties-1; i++)
        {
            c += ot_multipliers[i]->c_output[j];
        }
        timers["Triple computation"].stop();
        if (machine.amplify)
        {
            preamp_triples[j/n_amplify].a[j%n_amplify] = a;
            preamp_triples[j/n_amplify].b = b;
            preamp_triples[j/n_amplify].c[j%n_amplify] = c;
        }
        else if (machine.output)
        {
            timers["Writing"].start();
            a.output(output_file, false);
            b.output(output_file, false);
            c.output(output_file, false);
            timers["Writing"].stop();
        }
        else
        {
            plain_triples[j] = {{a, b, c}};
        }

#ifdef DEBUG_MASCOT
        cout << "lengths ";
        for (int i = 0; i < 3; i++)
            cout << value_bits[i].size() << " ";
        cout << endl;

        auto& P = global_player;
        SemiOpener<SemiShare<T>> MC;

        auto aa = MC.open(a, P);
        auto bb = MC.open(b, P);
        auto cc = MC.open(c, P);
        if (cc != aa * bb)
        {
            cout << j << " " << cc << " != " << aa << " * " << bb << ", diff " <<
                    (cc - aa * bb) << endl;
            cout << "OT output " << ot_multipliers[0]->c_output[j] << endl;
            assert(cc == aa * bb);
        }
#endif
    }

#ifdef DEBUG_MASCOT
    cout << "plain triple round done" << endl;
#endif
}

template<class U>
void SimpleMascotTripleGenerator<U>::generateTriples()
{
    CODE_LOCATION
    typedef typename U::OpenType T;

    auto& timers = this->timers;
    auto& machine = this->machine;
    auto& n_triples_per_loop = this->n_triples_per_loop;
    auto& value_bits = this->value_bits;
    auto& ot_multipliers = this->ot_multipliers;
    auto& nparties = this->nparties;
    auto& global_player = this->global_player;
    auto& nloops = this->nloops;
    auto& preamp_triples = this->preamp_triples;
    auto& output_file = this->output_file;
    auto& field_size = this->field_size;
    auto& n_preamp_triples_per_loop = this->n_preamp_triples_per_loop;
    auto& unchecked_triples = this->unchecked_triples;

	for (int i = 0; i < nparties-1; i++)
	    ot_multipliers[i]->inbox.push(DATA_TRIPLE);

    value_bits.resize(3);
    for (int i = 0; i < 2; i++)
        value_bits[2*i].resize(field_size * n_preamp_triples_per_loop);
    value_bits[1].resize(field_size * n_triples_per_loop);
    vector< PlainTriple<T,2> > amplified_triples;
    MacCheck MC(this->get_mac_key());

    if (machine.amplify)
        preamp_triples.resize(n_triples_per_loop);
    if (machine.generate_macs)
      {
	amplified_triples.resize(n_triples_per_loop);
      }

    unchecked_triples.resize(n_triples_per_loop);

    this->start_progress();

    for (int k = 0; k < nloops; k++)
    {
        this->plain_triple_round();

        if (machine.amplify)
        {
            PRNG G;
            if (machine.fiat_shamir and nparties == 2)
                ot_multipliers[0]->ot_correlator.common_seed(G);
            else
            {
                Octet seed[SEED_SIZE];
                create_random_seed(seed, global_player, SEED_SIZE);
                G.SetSeed(seed);
            }
            for (int i_triple = 0; i_triple < n_triples_per_loop; i_triple++)
            {
                PlainTriple<T,2> triple;
                triple.amplify(preamp_triples[i_triple], G);

                if (machine.generate_macs)
                    amplified_triples[i_triple] = triple;
                else if (machine.output)
                {
                    timers["Writing"].start();
                    triple.output(output_file);
                    timers["Writing"].stop();
                }
                else
                    for (int i = 0; i < 3; i++)
                        unchecked_triples[i_triple].by_index(i, 0).set_share(triple.by_index(i, 0));
            }

            if (machine.generate_macs)
            {
                for (int i_triple = 0; i_triple < n_triples_per_loop; i_triple++)
                    amplified_triples[i_triple].to(value_bits, i_triple,
                            machine.check ? 2 : 1);

                for (int i = 0; i < nparties-1; i++)
                    ot_multipliers[i]->inbox.push({});
                timers["Authentication OTs"].start();
                this->wait_for_multipliers();
                timers["Authentication OTs"].stop();

                for (int i_triple = 0; i_triple < n_triples_per_loop; i_triple++)
                {
                    unchecked_triples[i_triple].from(amplified_triples[i_triple], i_triple, *this);

                    if (!machine.check and machine.output)
                    {
                        timers["Writing"].start();
                        amplified_triples[i_triple].output(output_file);
                        timers["Writing"].stop();
                    }
                }

                if (machine.check)
                {
                    sacrifice(this->MC ? *this->MC : MC, G);
                }
            }
        }
    }
}

template<class T>
void MascotTripleGenerator<T>::sacrifice(typename T::MacCheck& MC, PRNG& G)
{
    CODE_LOCATION
    auto& machine = this->machine;
    auto& n_triples_per_loop = this->n_triples_per_loop;
    auto& global_player = this->global_player;
    auto& output_file = this->output_file;
    auto& unchecked_triples = this->unchecked_triples;

    check_field_size<typename T::clear>();

    vector<T> masked_as(n_triples_per_loop);
    vector<TripleToSacrifice<T> > masked_triples(n_triples_per_loop);
    for (int j = 0; j < n_triples_per_loop; j++)
    {
        masked_triples[j].template prepare_sacrifice<T>(unchecked_triples[j], G);
        masked_as[j] = masked_triples[j].a[0];
    }

    vector<OpenType> opened_as(n_triples_per_loop);
    MC.POpen_Begin(opened_as, masked_as, global_player);
    MC.POpen_End(opened_as, masked_as, global_player);

#ifdef DEBUG_MASCOT
    MC.Check(global_player);
    auto& P = global_player;

    for (int j = 0; j < n_triples_per_loop; j++)
        for (int i = 0; i < 2; i++)
        {
            auto a = MC.open(unchecked_triples[j].a[i], P);
            auto b = MC.open(unchecked_triples[j].b, P);
            auto c = MC.open(unchecked_triples[j].c[i], P);
            if (c != a * b)
            {
                cout << c << " != " << a << " * " << b << ", diff " << hex <<
                        (c - a * b) << endl;
                assert(c == a * b);
            }
        }

    MC.Check(global_player);
#endif

    for (int j = 0; j < n_triples_per_loop; j++) {
        MC.AddToCheck(masked_triples[j].compute_check_share(opened_as[j]), 0,
                global_player);
    }

    MC.Check(global_player);

    if (machine.generate_bits)
        generate_bits_from_triples(MC, output_file, typename T::clear());
    else
        if (machine.output)
            for (int j = 0; j < n_triples_per_loop; j++)
                unchecked_triples[j].output(output_file, 1);
}

template<class W>
template<class U>
void Spdz2kTripleGenerator<W>::sacrifice_z2k(U& MC, PRNG& G)
{
    CODE_LOCATION
    typedef SacriType T;
    typedef OpenType V;
    typedef typename W::PrepCheckType PrepCheckType;

    auto& machine = this->machine;
    auto& n_triples_per_loop = this->n_triples_per_loop;
    auto& global_player = this->global_player;
    auto& output_file = this->output_file;
    auto& unchecked_triples = this->unchecked_triples;

    vector<PrepCheckType> masked_as(n_triples_per_loop);
    vector<TripleToSacrifice<Share<T>> > masked_triples(n_triples_per_loop);
    for (int j = 0; j < n_triples_per_loop; j++)
    {
        // compute [p] = t * [a] - [ahat]
        // and first part of [sigma], i.e., t * [c] - [chat] 
        masked_triples[j].template prepare_sacrifice<W>(unchecked_triples[j], G);
        masked_as[j] = masked_triples[j].a[0];
        // enough randomness in values
        MC.set_random_element({});
    }

    vector<T> opened_as(n_triples_per_loop);
    MC.POpen_Begin(opened_as, masked_as, global_player);
    MC.POpen_End(opened_as, masked_as, global_player);

    vector<PrepCheckType> sigmas;
    for (int j = 0; j < n_triples_per_loop; j++) {
        // compute t * [c] - [chat] - [b] * p
        sigmas.push_back(masked_triples[j].compute_check_share(V(opened_as[j])));
        // enough randomness in values
        MC.set_random_element({});
    }
    vector<T> open_sigmas;
    
    MC.POpen_Begin(open_sigmas, sigmas, global_player);
    MC.POpen_End(open_sigmas, sigmas, global_player);
    MC.Check(global_player);

    for (int j = 0; j < n_triples_per_loop; j++) {
        if (V(open_sigmas[j]) != 0)
        {
            throw runtime_error("sacrifice fail");
        }
    }
    
    if (machine.generate_bits)
        throw NotImplemented();
    else
        if (machine.output)
            for (int j = 0; j < n_triples_per_loop; j++)
                unchecked_triples[j].template reduce<W>().output(output_file, 1);
}

template<class T>
template<int X, int L>
void MascotTripleGenerator<T>::generate_bits_from_triples(MacCheck& MC,
        ofstream& output_file, Gfp_<X, L>)
{
    CODE_LOCATION
    typedef Gfp_<X, L> Gfp1;
    auto& triples = this->unchecked_triples;
    auto& n_triples_per_loop = this->n_triples_per_loop;
    auto& global_player = this->global_player;
    vector< Share<Gfp1> > a_plus_b(n_triples_per_loop), a_squared(n_triples_per_loop);
    for (int i = 0; i < n_triples_per_loop; i++)
        a_plus_b[i] = triples[i].a[0] + triples[i].b;
    vector<Gfp1> opened(n_triples_per_loop);
    MC.POpen_Begin(opened, a_plus_b, global_player);
    MC.POpen_End(opened, a_plus_b, global_player);
    for (int i = 0; i < n_triples_per_loop; i++)
        a_squared[i] = triples[i].a[0] * opened[i] - triples[i].c[0];
    MC.POpen_Begin(opened, a_squared, global_player);
    MC.POpen_End(opened, a_squared, global_player);
    MC.Check(global_player);
    auto one = Share<Gfp1>::constant(1, global_player.my_num(), MC.get_alphai());
    bits.clear();
    for (int i = 0; i < n_triples_per_loop; i++)
    {
        Gfp1 root = opened[i].square_root();
        if (root.is_zero())
            continue;
        Share<Gfp1> bit = (triples[i].a[0] / root + one) / Gfp1(2);
        if (this->machine.output)
            bit.output(output_file, false);
        else
            bits.push_back(bit);
    }
}

template<class T>
template<class U>
void MascotTripleGenerator<T>::generate_bits_from_triples(MacCheck&, ofstream&, U)
{
    throw HowWouldThatWork();
}

template <class T>
void OtTripleGenerator<T>::start_progress()
{
    wait_for_multipliers();
    lock();
    signal();
    wait();
    gettimeofday(&last_lap, 0);
}

template<class T>
void OtTripleGenerator<T>::print_progress(int k)
{
    if (thread_num == 0 && my_num == 0)
    {
        struct timeval stop;
        gettimeofday(&stop, 0);
        if (timeval_diff_in_seconds(&last_lap, &stop) > 1)
        {
            double diff = timeval_diff_in_seconds(&machine.start, &stop);
            double throughput = k * n_triples_per_loop * machine.nthreads / diff;
            double remaining = diff * (nloops - k) / k;
            cout << k << '/' << nloops << ", throughput: " << throughput
                    << ", time left: " << remaining << ", elapsed: " << diff
                    << ", estimated total: " << (diff + remaining) << endl;
            last_lap = stop;
        }
    }
}

inline
void GeneratorThread::lock()
{
    pthread_mutex_lock(&mutex);
}

inline
void GeneratorThread::unlock()
{
    pthread_mutex_unlock(&mutex);
}

inline
void GeneratorThread::signal()
{
    pthread_cond_signal(&ready);
}

inline
void GeneratorThread::wait()
{
    if (multi_threaded)
        pthread_cond_wait(&ready, &mutex);
}

template<class T>
void OtTripleGenerator<T>::signal_multipliers(MultJob job)
{
    for (int i = 0; i < nparties-1; i++)
        ot_multipliers[i]->inbox.push(job);
}

template<class T>
void OtTripleGenerator<T>::wait_for_multipliers()
{
    for (int i = 0; i < nparties-1; i++)
        ot_multipliers[i]->outbox.pop();
}

template<class T>
void OtTripleGenerator<T>::run_multipliers(MultJob job)
{
    signal_multipliers(job);
    wait_for_multipliers();
}
