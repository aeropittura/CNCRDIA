#include "ot/ot_extension.h"
#include "ot/ot_extension_with_matrix.h"
#include "ot/ot_triple_setup.h"

#include "ot/tools.h"
#include "numeric/gf2n.h"
#include "support/aes.h"
#include "support/mmo.h"
#include "support/intrinsics.h"
#include "support/benchmarking.h"


OtExtension::OtExtension(const BaseOT& base_ot, TwoPartyPlayer* player,
        bool passive) : player(player)
{
    ot_role = INV_ROLE(base_ot.ot_role);
    passive_only = passive;
    init(base_ot.receiver_inputs, base_ot.sender_inputs, base_ot.receiver_outputs);
}

void OtExtension::init(const OtTripleSetup& setup, int index)
{
    init(setup.base_receiver_inputs, setup.base_sender_inputs.at(index),
            setup.base_receiver_outputs.at(index));
}


// test if a == b
int eq_m128i(__m128i a, __m128i b)
{
    __m128i vcmp = _mm_cmpeq_epi8(a, b);
    uint16_t vmask = _mm_movemask_epi8(vcmp);
    return (vmask == 0xffff);
}


bool OtExtensionWithMatrix::warned = false;

void OtExtensionWithMatrix::check_correlation(int n_ots,
    const BitVector& receiver_input)
{
    if (not warned)
    {
        insecure("OT extension (security of KOS15 is unclear, "
                "see https://eprint.iacr.org/2022/192.)");
        warned = true;
    }

    //cout << "\tStarting correlation check\n" << flush;
#ifdef OTEXT_TIMER
    timeval startv, endv;
    gettimeofday(&startv, NULL);
#endif
    if (nbase_ots != 128)
    {
        cerr << "Correlation check not implemented for length != 128\n";
        throw NotImplemented();
    }
    GlobalPrng G(*player);
#ifdef OTEXT_TIMER
    gettimeofday(&endv, NULL);
    double elapsed = timeval_diff(&startv, &endv);
    cout << "\t\tCommitment for seed took time " << elapsed/1000000 << endl << flush;
    times["Commitment for seed"] += timeval_diff(&startv, &endv);
    gettimeofday(&startv, NULL);
#endif

    __m128i Delta, x128i;
    Delta = _mm_load_si128((__m128i*)&(base_receiver_input.get_ptr()[0]));

    BitVector chi(nbase_ots);
    BitVector x(nbase_ots);
    __m128i t = _mm_setzero_si128();
    __m128i q = _mm_setzero_si128();
    __m128i t2 = _mm_setzero_si128();
    __m128i q2 = _mm_setzero_si128();
    __m128i chii, ti, qi, ti2, qi2;
    x128i = _mm_setzero_si128();

    for (int i = 0; i < n_ots; i++)
    {
//        chi.randomize(G);
//        chii = _mm_load_si128((__m128i*)&(chi.get_ptr()[0]));
        chii = G.get_doubleword();

        if (ot_role & RECEIVER)
        {
            if (receiver_input.get_bit(i) == 1)
            {
                x128i = _mm_xor_si128(x128i, chii);
            }
            ti = _mm_loadu_si128((__m128i*)get_receiver_output(i));
            // multiply over polynomial ring to avoid reduction
            mul128(ti, chii, &ti, &ti2);
            t = _mm_xor_si128(t, ti);
            t2 = _mm_xor_si128(t2, ti2);
        }
        if (ot_role & SENDER)
        {
            qi = _mm_loadu_si128((__m128i*)(get_sender_output(0, i)));
            mul128(qi, chii, &qi, &qi2);
            q = _mm_xor_si128(q, qi);
            q2 = _mm_xor_si128(q2, qi2);
        }
    }
#ifdef OTEXT_DEBUG
    if (ot_role & RECEIVER)
    {
        cout << "\tSending x,t\n";
        cout << "\tsend x = " << __m128i_toString<Octet>(x128i) << endl;
        cout << "\tsend t = " << __m128i_toString<Octet>(t) << endl;
        cout << "\tsend t2 = " << __m128i_toString<Octet>(t2) << endl;
    }
#endif
    check_iteration(Delta, q, q2, t, t2, x128i);
#ifdef OTEXT_TIMER
    gettimeofday(&endv, NULL);
    elapsed = timeval_diff(&startv, &endv);
    cout << "\t\tChecking correlation took time " << elapsed/1000000 << endl << flush;
    times["Checking correlation"] += timeval_diff(&startv, &endv);
#endif
}

void OtExtensionWithMatrix::check_iteration(__m128i delta, __m128i q, __m128i q2,
    __m128i t, __m128i t2, __m128i x)
{
    vector<OctetStream> os(2);
    // send x, t;
    __m128i received_t, received_t2, received_x, tmp1, tmp2;
    if (ot_role & RECEIVER)
    {
        os[0].append((Octet*)&x, sizeof(x));
        os[0].append((Octet*)&t, sizeof(t));
        os[0].append((Octet*)&t2, sizeof(t2));
    }
    send_if_ot_receiver(player, os, ot_role);

    if (ot_role & SENDER)
    {
        os[1].consume((Octet*)&received_x, sizeof(received_x));
        os[1].consume((Octet*)&received_t, sizeof(received_t));
        os[1].consume((Octet*)&received_t2, sizeof(received_t2));

        // check t = x * Delta + q
        //gfmul128(received_x, delta, &tmp1);
        mul128(received_x, delta, &tmp1, &tmp2);
        tmp1 = _mm_xor_si128(tmp1, q);
        tmp2 = _mm_xor_si128(tmp2, q2);

        if (eq_m128i(tmp1, received_t) && eq_m128i(tmp2, received_t2))
        {
            //cout << "\tCheck passed\n";
        }
        else
        {
            cerr << "Correlation check failed\n";
            cout << "rec t = " << __m128i_toString<Octet>(received_t) << endl;
            cout << "tmp1  = " << __m128i_toString<Octet>(tmp1) << endl;
            cout << "q  = " << __m128i_toString<Octet>(q) << endl;
            throw runtime_error("correlation check");
        }
    }
}
