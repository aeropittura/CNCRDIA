/*
 * protocols/brain/brain_prep.hpp
 *
 */

#pragma once

#include "protocols/brain/brain_prep.h"
#include "vm/processor.h"
#include "protocols/replicated/malicious_rep_mc.h"
#include "support/subroutines.h"
#include "numeric/gfp.h"

template<class T> class ZProtocol;

template<int L>
class Zint : public SignedZ2<L + 2>
{
    typedef SignedZ2<L + 2> Super;

public:
    static string type_string()
    {
        return "Zint" + to_string(L);
    }

    Zint()
    {
    }

    template<class T>
    Zint(const T& other) : Super(other)
    {
    }

    Zint& operator+=(const Zint& other)
    {
        return *this = *this + other;
    }

    Zint& operator-=(const Zint& other)
    {
        return *this = *this - other;
    }

    // optimized Rep3
    Zint& operator+=(PRNG& G)
    {
        return *this = *this + G.get<Zint>();
    }

    Zint& operator-=(PRNG& G)
    {
        return *this = *this - G.get<Zint>();
    }

    void randomize(PRNG& G, int n = -1)
    {
        (void) n;
        *this = G.get<Z2<L>>();
    }
};

template<class U>
class ZShare : public Rep3Share<Zint<U::Z_BITS>>
{
public:
    typedef ZProtocol<U> Protocol;
    typedef ReplicatedOpener<ZShare> MacCheck;

    ZShare()
    {
    }

    template<class T>
    ZShare(const FixedVec<T, 2>& other)
    {
        FixedVec<T, 2>::operator=(other);
    }
};

template<class U>
class ZProtocol : public Replicated<Rep3Share<Zint<U::Z_BITS>>>
{
    typedef Rep3Share<Zint<U::Z_BITS>> T;
    vector<T> random;
    SeededPrng G;
    ReplicatedInput<Rep3Share<Zint<U::N_MASK_BITS>>> input;

public:
    ZProtocol(Player& P) : Replicated<T>(P), input(0, this->P)
    {
    }

    T get_random()
    {
        if (random.empty())
        {
            int buffer_size = OnlineOptions::singleton.batch_size;
            input.reset_all(this->P);
            for (int i = 0; i < buffer_size; i++)
            {
                typename U::clear tmp;
                tmp.randomize(G);
                input.add_mine(tmp);
            }
            for (int i = 0; i < this->P.num_players(); i++)
                input.add_other(i);
            input.exchange();
            for (int i = 0; i < buffer_size; i++)
            {
                random.push_back({});
                for (int j = 0; j < 3; j++)
                    random.back() += input.finalize(j);
            }
        }

        auto res = random.back();
        random.pop_back();
        return res;
    }
};

template<class T>
void BrainPrep<T>::basic_setup(Player&)
{
    Gfp2::initialize_default(DIV_CEIL(T::Z_BITS + 3, 8) * 8);
}

template<class T>
void BrainPrep<T>::buffer_triples()
{
    CODE_LOCATION
    if(Gfp2::get_zp_d().pr_bit_length
            <= ZProtocol<T>::ShareType::clear::N_BITS)
        throw runtime_error(
                to_string(Gfp2::get_zp_d().pr_bit_length)
                        + "-bit prime too short for "
                        + to_string(ZProtocol<T>::ShareType::clear::N_BITS)
                        + "-bit integer computation");
    typedef Rep3Share<Gfp2> PShare;
    auto buffer_size = BaseMachine::batch_size<T>(DATA_TRIPLE);
    Player& P = this->protocol->P;
    vector<array<ZShare<T>, 3>> triples;
    vector<array<Rep3Share<Gfp2>, 3>> check_triples;
    DataPositions usage;
    HashMaliciousRepOpener<PShare> MC;
    vector<Rep3Share<Gfp2>> masked, checks;
    vector<Gfp2> opened;
    ZProtocol<T> z_protocol(P);
    Replicated<PShare> p_protocol(P);
    generate_triples(triples, buffer_size, &z_protocol);
    check_triples.resize(buffer_size);
    p_protocol.initialize_mult();
    for (int i = 0; i < buffer_size; i++)
    {
        PShare a = p_protocol.get_random();
        PShare b = triples[i][1];
        p_protocol.prepare_mul(a, b);
        check_triples[i][0] = a;
        check_triples[i][1] = b;
    }
    p_protocol.exchange();
    auto t = create_random<Gfp2>(P);
    for (int i = 0; i < buffer_size; i++)
    {
        check_triples[i][2] = p_protocol.finalize_mul();
        PShare a = triples[i][0];
        auto& f = check_triples[i][0];
        masked.push_back(a * t - f);
    }
    MC.POpen(opened, masked, P);
    for (int i = 0; i < buffer_size; i++)
    {
        auto& b = check_triples[i][1];
        PShare c = triples[i][2];
        auto& h = check_triples[i][2];
        auto& rho = opened[i];
        checks.push_back(t * c - h - rho * b);
    }
    MC.check_for(0, checks, P);
    MC.Check(P);
    for (auto& x : triples)
        this->triples.push_back({{x[0], x[1], x[2]}});
}

template<class T>
void BrainPrep<T>::buffer_inputs(int player)
{
    this->buffer_inputs_as_usual(player, this->proc);
}
