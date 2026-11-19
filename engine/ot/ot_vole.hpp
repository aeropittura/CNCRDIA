#pragma once

#include "ot/config.h"

#include "ot/ot_vole.h"
#include "support/oct.h"
#include "support/subroutines.h"

//#define OTVOLE_TIMER

template <class T>
void OtVoleBase<T>::evaluate(vector<T>& output, const vector<T>& new_receiver_input) {
    CODE_LOCATION
    if (OnlineOptions::singleton.has_option("verbose_vole"))
        fprintf(stderr, "%d-bit VOLE with %zu elements and S=%d\n", T::N_BITS,
                new_receiver_input.size(), S);

    const int N1 = new_receiver_input.size() + 1;
    output.resize(new_receiver_input.size());
    auto& os = oss;
    os.resize(2);
    os[0].reset_write_head();
    os[1].reset_write_head();

    if (this->ot_role & SENDER) {
        T extra;
        extra.randomize(local_prng);
        corr_prime.rows = new_receiver_input;
        corr_prime.rows.push_back(extra);
        for (int i = 0; i < S; ++i)
        {
            t0[i].randomize(this->g_sender[i][0], N1);
            t1[i].randomize(this->g_sender[i][1], N1);
            (corr_prime + t1[i] + t0[i]).pack(os[0]);
        }
    }
    send_if_ot_sender(this->player, os, this->ot_role);
    if (this->ot_role & RECEIVER) {
        for (int i = 0; i < S; ++i)
        {
            t[i].randomize(this->g_receiver[i], N1);

            int choice_bit = this->base_receiver_input.get_bit(i);
            if (choice_bit == 1) {
                u[i].unpack(os[1]);
                a[i] = u[i] - t[i];
            } else {
                a[i] = t[i];
                u[i].unpack(os[1]);
            }
        }
    }
    os[0].reset_write_head();
    os[1].reset_write_head();

    if (!this->passive_only) {
        this->consistency_check(os);
    }

    Row<T> res(N1);
    if (this->ot_role & RECEIVER) {
        for (int i = 0; i < S; ++i)
            res += (a[i] << i);
    }
    if (this->ot_role & SENDER) {
        for (int i = 0; i < S; ++i)
            res -= (t0[i] << i);
    }
    for (int i = 0; i < N1 - 1; ++i)
        output[i] = res.rows[i];
}

template <class T>
void OtVoleBase<T>::evaluate(vector<T>& output, int nValues, const BitVector& new_receiver_input) {
    vector<T> values(nValues);
    if (ot_role & SENDER)
    {
        if (new_receiver_input.size() != (size_t) nValues * 8 * T::N_BYTES)
            throw InvalidLength();
        for (int i = 0; i < nValues; ++i)
            values[i] = T(new_receiver_input.get_pointer_to_byte(i, T::N_BYTES));
    }
    evaluate(output, values);
}

template <class T>
void OtVoleBase<T>::set_coefficients(__m128i* coefficients, PRNG& G, int num_blocks) const {
    G.get_octets((Octet*) coefficients, num_blocks * sizeof(__m128i));
}

template <class T>
template<class U>
void OtVoleBase<T>::hash_row(OctetStream& os, const U& row,
        const __m128i* coefficients)
{
	Octet hash[VOLE_HASH_SIZE] = {0};
	this->hash_row(hash, row, coefficients);
	os.append(hash, VOLE_HASH_SIZE);
}

template <class T>
template <class U>
void OtVoleBase<T>::hash_row(Octet* hash, const U& row,
        const __m128i* coefficients)
{
	__m128i res[2];
	for (int i = 0; i < 2; i++)
		res[i] = _mm_setzero_si128();
	hash_row(res, row, coefficients);

	crypto_generichash(hash, crypto_generichash_BYTES,
			(Octet*) res, crypto_generichash_BYTES, NULL, 0);
}

template <class T>
template <class U>
void OtVoleBase<T>::hash_row(__m128i res[2], const U& row,
        const __m128i* coefficients)
{
	auto coefficient_base = coefficients;
	int num_blocks = DIV_CEIL(row.size() * T::size(), 16);
	__m128i buffer[T::N_BYTES];
	size_t next = 0;
	while (next + 16 <= row.size())
	{
		for (int j = 0; j < 16; j++)
			memcpy((char*) buffer + j * T::size(), row[next++].get_ptr(), T::size());
		for (int j = 0; j < T::size(); j++)
			add_mult(res, buffer[j], *coefficients++);
	}
	for (int j = 0; j < 16; j++)
		if (next < row.size())
			memcpy((char*) buffer + j * T::size(), row[next++].get_ptr(), T::size());
		else
		    memset((char*) buffer + j * T::size(), 0, T::size());
	for (int j = 0; j < num_blocks % T::size(); j++)
		add_mult(res, buffer[j], *coefficients++);
	assert(coefficients == coefficient_base + num_blocks);
}

template <class T>
void OtVoleBase<T>::add_mult(__m128i res[2], __m128i a, __m128i b)
{
	__m128i prods[2];
	mul128(a, b, &prods[0], &prods[1]);
	res[0] ^= prods[0];
	res[1] ^= prods[1];
}

template <>
template <class U>
inline
void OtVoleBase<Z2<128>>::hash_row(__m128i res[2],
		const U& row, const __m128i* coefficients)
{
	for (size_t i = 0; i < row.size(); i++)
	{
		__m128i block = Int128(row[i].get_limb(1), row[i].get_limb(0)).a;
		add_mult(res, block, coefficients[i]);
	}
}

template <>
template <class U>
inline
void OtVoleBase<Z2<192>>::hash_row(__m128i res[2], const U& row,
		const __m128i* coefficients)
{
	__m128i block;
	size_t j;
	for (j = 0; j < row.size() - 1; j += 2)
	{
		auto x = row[j];
		auto y = row[j + 1];
		block = Int128(x.get_limb(1), x.get_limb(0)).a;
		add_mult(res, block, *coefficients++);
		block = Int128(y.get_limb(0), x.get_limb(2)).a;
		add_mult(res, block, *coefficients++);
		block = Int128(y.get_limb(2), y.get_limb(1)).a;
		add_mult(res, block, *coefficients++);
	}
	if (j < row.size())
	{
		auto x = row[j];
		block = Int128(x.get_limb(1), x.get_limb(0)).a;
		add_mult(res, block, *coefficients++);
		block = Int128(x.get_limb(2)).a;
		add_mult(res, block, *coefficients++);
	}
}

template <class T>
void OtVoleBase<T>::consistency_check(vector<OctetStream>& os) {
	PRNG coef_prng_sender;
	PRNG coef_prng_receiver;

    if (this->ot_role & RECEIVER) {
    	coef_prng_receiver.re_seed();
    	os[0].append(coef_prng_receiver.get_seed(), SEED_SIZE);
    }
    send_if_ot_receiver(this->player, os, this->ot_role);
    if (this->ot_role & SENDER) {
        Octet seed[SEED_SIZE];
        os[1].consume(seed, SEED_SIZE);
        coef_prng_sender.SetSeed(seed);
    }
    os[0].reset_write_head();
    os[1].reset_write_head();

    if (this->ot_role & SENDER) {
#ifdef OTVOLE_TIMER
        timeval totalstartv, totalendv;
        gettimeofday(&totalstartv, NULL);
#endif
        int total_bytes = t0[0].size() * T::size();
        int num_blocks = (total_bytes) / 16 + ((total_bytes % 16) != 0);
        __m128i* coefficients = new __m128i[num_blocks];
        this->set_coefficients(coefficients, coef_prng_sender, num_blocks);

        for (int alpha = 0; alpha < S; ++alpha)
        {
            for (int i = 0; i < n_challenges(); i++)
            {
                int beta = get_challenge(coef_prng_sender, i);

                auto t00 = t0[alpha] - t0[beta];
                auto t01 = t0[alpha] - t1[beta];
                auto t10 = t1[alpha] - t0[beta];
                auto t11 = t1[alpha] - t1[beta];

                this->hash_row(os[0], t00, coefficients);
                this->hash_row(os[0], t01, coefficients);
                this->hash_row(os[0], t10, coefficients);
                this->hash_row(os[0], t11, coefficients);
            }
        }
        delete[] coefficients;
#ifdef OTVOLE_TIMER
        gettimeofday(&totalendv, NULL);
        double elapsed = timeval_diff(&totalstartv, &totalendv);
        cout << "\t\tCheck time sender: " << elapsed/1000000 << endl << flush;
#endif
    }
    
    send_if_ot_sender(this->player, os, this->ot_role);
    if (this->ot_role & RECEIVER) {
#ifdef OTVOLE_TIMER
        timeval totalstartv, totalendv;
        gettimeofday(&totalstartv, NULL);
#endif
        int total_bytes = t[0].size() * T::size();
        int num_blocks = (total_bytes) / 16 + ((total_bytes % 16) != 0);
        __m128i* coefficients = new __m128i[num_blocks];
        this->set_coefficients(coefficients, coef_prng_receiver, num_blocks);

        Octet h00[VOLE_HASH_SIZE] = {0};
        Octet h01[VOLE_HASH_SIZE] = {0};
        Octet h10[VOLE_HASH_SIZE] = {0};
        Octet h11[VOLE_HASH_SIZE] = {0};
        vector<vector<Octet*>> hashes(2);
        hashes[0] = {h00, h01};
        hashes[1] = {h10, h11};

        for (int alpha = 0; alpha < S; ++alpha)
        {
            for (int i = 0; i < n_challenges(); i++)
            {
                int beta = get_challenge(coef_prng_receiver, i);

                os[1].consume(hashes[0][0], VOLE_HASH_SIZE);
                os[1].consume(hashes[0][1], VOLE_HASH_SIZE);
                os[1].consume(hashes[1][0], VOLE_HASH_SIZE);
                os[1].consume(hashes[1][1], VOLE_HASH_SIZE);

                int choice_alpha = this->base_receiver_input.get_bit(alpha);
                int choice_beta = this->base_receiver_input.get_bit(beta);

                auto diff = t[alpha] - t[beta];
                Octet* choice_hash = hashes[choice_alpha][choice_beta];
                Octet diff_t[VOLE_HASH_SIZE] = {0};
                this->hash_row(diff_t, diff, coefficients);
                
                Octet* not_choice_hash = hashes[1 - choice_alpha][1 - choice_beta];
                Octet other_diff[VOLE_HASH_SIZE] = {0};
                auto a = u[alpha] - u[beta];
                auto b = a - t[alpha];
                auto tmp = b + t[beta];
                this->hash_row(other_diff, tmp, coefficients);
                
                if (!OCTETS_EQUAL(choice_hash, diff_t, VOLE_HASH_SIZE)) {
                    throw ConsistencyCheckFail();
                }
                if (!OCTETS_EQUAL(not_choice_hash, other_diff, VOLE_HASH_SIZE)) {
                    throw ConsistencyCheckFail();
                }
                if (alpha != beta && u[alpha] == u[beta]) {
                    throw ConsistencyCheckFail();
                }
            }
        }
        delete[] coefficients;
#ifdef OTVOLE_TIMER
        gettimeofday(&totalendv, NULL);
        double elapsed = timeval_diff(&totalstartv, &totalendv);
        cout << "\t\tCheck receiver: " << elapsed/1000000 << endl << flush;
#endif
    }
}
