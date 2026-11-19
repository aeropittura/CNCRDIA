
#pragma once

#include "numeric/z2k.h"
#include "ot/ot_extension.h"
#include "ot/row.h"
#include "ot/config.h"

using namespace std;

template <class T>
class OtVoleBase : public OtExtension
{
public:
	const int S;

	OtVoleBase(int S,
	                TwoPartyPlayer* player,
	                OT_ROLE role=BOTH,
	                bool passive=false)
	    : OtExtension(player, role, passive),
	        S(S),
	        corr_prime(),
	        t0(S),
	        t1(S),
	        u(S),
	        t(S),
	        a(S) {
	            // need to flip roles for OT extension init, reset to original role here
	            this->ot_role = role;
	            local_prng.re_seed();
	        }

	    virtual ~OtVoleBase() {}

	    void evaluate(vector<T>& output, const vector<T>& new_receiver_input);

	    void evaluate(vector<T>& output, int nValues, const BitVector& new_receiver_input);

	    virtual int n_challenges() { return S; }
	    virtual int get_challenge(PRNG&, int i) { return i; }

	protected:

		// Sender fields
		Row<T> corr_prime;
		vector<Row<T>> t0, t1;
		// Receiver fields
		vector<Row<T>> u, t, a;
		// Both
		PRNG local_prng;

	    vector<OctetStream> oss;

	    virtual void consistency_check (vector<OctetStream>& os);

	    void set_coefficients(__m128i* coefficients, PRNG& G, int num_elements) const;

	    template<class U>
	    void hash_row(OctetStream& os, const U& row, const __m128i* coefficients);
	    template<class U>
	    void hash_row(Octet* hash, const U& row, const __m128i* coefficients);
	    template<class U>
	    void hash_row(__m128i res[2], const U& row, const __m128i* coefficients);

	    static void add_mult(__m128i res[2], __m128i a, __m128i b);
};

template <class T>
class OtVole : public OtVoleBase<T>
{

public:
    OtVole(int S,
                TwoPartyPlayer* player,
                OT_ROLE role=BOTH,
                bool passive=false)
    : OtVoleBase<T>(S, player, role, passive) {
    }

    int n_challenges() { return NUM_VOLE_CHALLENGES; }
    int get_challenge(PRNG& G, int) { return G.get_uint(this->S); }
};
