/*
 * he/offline/dist_key_gen.cpp
 *
 */

#include "he/offline/dist_key_gen.h"
#include "support/subroutines.h"

#include "numeric/modp.hpp"

/*
 * This creates the "pseudo-encryption" of the R_q element mess,
 *   - As required for key switching.
 */
void Encrypt_Rq_Element(Ciphertext& c,const RqElement& mess, const RandomCoins& rc,
        const FHE_PK& pk)
{
    auto c1 = pk.a() * rc.u() + rc.v() * pk.p();
    auto c0 = pk.b() * rc.u();
    auto edd = rc.w() * pk.p();

    edd.change_rep(evaluation,evaluation);
    add(edd,edd,mess);
    add(c0,c0,edd);
    c.set(c0,c1,pk);
}


void DistKeyGenerator::fake(FHE_PK& pk, vector<FHE_SK>& sks, const Bigint& p, int nparties)
{
    vector<DistKeyGenerator> dkgs(nparties, {pk.get_parameters(), p});
    sks.resize(nparties, {pk.get_parameters(), p});
    vector<RqElement> secrets(nparties, pk.get_parameters());
    PRNG G;
    G.re_seed();
    for (int i = 0; i < nparties; i++)
        dkgs[i].gen_random_data(G);

    RqElement a = dkgs[0].a;
    for (size_t i = 1; i < dkgs.size(); i++)
        a += dkgs[i].a;
    for (int i = 0; i < nparties; i++)
    {
        dkgs[i].a = a;
        dkgs[i].compute_b();
        secrets[i] = dkgs[i].secret;
    }

    RqElement b = dkgs[0].b;
    for (size_t i = 1; i < dkgs.size(); i++)
        b += dkgs[i].b;
    pk.check_noise(b - a * sum(secrets));
    for (int i = 0; i < nparties; i++)
    {
        dkgs[i].b = b;
        dkgs[i].compute_enc_dash();
        cout << "after enc_dash" << endl;
        pk.check_noise(dkgs[i].b - dkgs[i].a * sum(secrets));
    }

    Ciphertext enc_dash = dkgs[0].enc_dash;
    for (size_t i = 1; i < dkgs.size(); i++)
        enc_dash += dkgs[i].enc_dash;
    for (int i = 0; i < nparties; i++)
    {
        dkgs[i].enc_dash = enc_dash;
        dkgs[i].compute_enc();
        cout << "after enc" << endl;
        pk.check_noise(dkgs[i].b - dkgs[i].a * sum(secrets));
    }

    RqElement s = sum(secrets);
    s.lower_level();
    FHE_SK sk(pk.get_parameters(), p);
    sk.assign(s);

    Ciphertext enc = dkgs[0].enc;
    for (size_t i = 1; i < dkgs.size(); i++)
        enc += dkgs[i].enc;
    for (int i = 0; i < nparties; i++)
    {
        dkgs[i].enc = enc;
        dkgs[i].check_equality(dkgs[0]);
        dkgs[i].finalize(pk, sks[i]);
        cout << "after finalize" << endl;
        pk.check_noise(dkgs[i].b - dkgs[i].a * sum(secrets));
        pk.check_noise(pk.b() - pk.a() * sum(secrets));
        pk.check_noise(sk);
    }

    cout << "final" << endl;
    pk.check_noise(sum(sks));
}


DistKeyGenerator::DistKeyGenerator(const FheParameters& params, const Bigint& p) :
        params(params), rc1(params), rc2(params), secret(params, evaluation,
                evaluation), a(params, evaluation, evaluation), e(params,
                evaluation, evaluation), b(params, evaluation, evaluation), pk(
                params, p), enc_dash(params), enc(params), p(p)
{
}

/*
 * Generate the random data used in key generation
 */
void DistKeyGenerator::gen_random_data(PRNG& G)
{
    secret.from(GaussianGenerator<Bigint>(params.get_dg(), G));
    rc1.generate(G);
    rc2.generate(G);
    a.randomize(G);
    e.from(GaussianGenerator<Bigint>(params.get_dg(), G));
}

DistKeyGenerator& DistKeyGenerator::operator+=(const DistKeyGenerator& other)
{
    secret += other.secret;
    a += other.a;
    e += other.e;
    return *this;
}

void DistKeyGenerator::sum_a(const vector<RqElement>& as)
{
    a = sum(as);
}

void DistKeyGenerator::compute_b()
{
    mul(b, a, secret);
    mul(e, e, p);
    add(b, b, e);
    pk.check_noise(b - a * secret);
}

void DistKeyGenerator::compute_enc_dash(const vector<RqElement>& bs)
{
    b = sum(bs);
    compute_enc_dash();
}

void DistKeyGenerator::compute_enc_dash()
{
    RqElement dummy(params);
    pk.assign(a,b,dummy,dummy);

    secret.raise_level();
    RqElement ps(secret);
    ps.mult_by_p1();
    ps.negate();
    Encrypt_Rq_Element(enc_dash, ps, rc1, pk);
}

void DistKeyGenerator::compute_enc(const vector<Ciphertext>& enc_dashs)
{
    enc_dash = sum(enc_dashs);
    compute_enc();
}

void DistKeyGenerator::compute_enc()
{
    Ciphertext enc_zero(params);
    RqElement zero(RqElement(params.FFTD(), polynomial,polynomial));
    zero.assign_zero();
    pk.quasi_encrypt(enc_zero, zero, rc2);

    // Create global key-switching data
    RqElement e0(enc_dash.c0());
    RqElement e1(enc_dash.c1());
    mul(e0, e0, secret);
    mul(e1, e1, secret);
    enc.set(e0, e1, pk);

    add(enc, enc, enc_zero);
}

void DistKeyGenerator::sum_enc(const vector<Ciphertext>& encs)
{
    enc = sum(encs);
}

void DistKeyGenerator::finalize(FHE_PK& pk, FHE_SK& sk)
{
    pk.assign(a, b, enc.c1(), enc.c0());
    secret.lower_level();
    sk.assign(secret);
}

void DistKeyGenerator::check_equality(const DistKeyGenerator& other)
{
    if (a != other.a)
        throw runtime_error("no match at a");
    if (b != other.b)
        throw runtime_error("no match at b");
    if (enc_dash != other.enc_dash)
        throw runtime_error("no match at enc_dash");
    if (enc != other.enc)
        throw runtime_error("no match at enc");
}


/*
 * Simulate an execution of key_gen to check that these nplayers seeds
 * produce the correct public key.
 */
void check_randomness(vector<OctetStream>& seeds,
                      const Ciphertext& actual_sw,const FHE_PK& pk,
                      const Ciphertext& enc_dash,
                      int num_players)
{
    const FheParameters& params=actual_sw.get_parameters();

    vector<DistKeyGenerator> player_keys(num_players, {params, pk.p()});
    DistKeyGenerator global_key(params, pk.p());

    RqElement b(RqElement(params.FFTD(), evaluation, evaluation));
    RqElement zero(RqElement(params.FFTD(), polynomial,polynomial));
    zero.assign_zero();

    Ciphertext ed(params), ed_sum(params), ezero(params), ezero_sum(params);
    PRNG G;

    // Re-create the randomness from these seeds
    for (int i = 0; i < num_players; i++)
      { G.SetSeed(seeds[i].get_data());
#ifdef VERBOSE_KEYGEN
        cout << "\tSeed for player " << i << " is..." << seeds[i] << endl;
#endif
        player_keys[i].gen_random_data(G);
        global_key += player_keys[i];
      }
    mul(b, global_key.a, global_key.secret);
    mul(global_key.e, global_key.e, pk.p());
    add(b, b, global_key.e);

    // Check the main pk (a,b)
    if (!pk.a().equals(global_key.a))
        throw BadKeygen("a doesn't match");
    if (!pk.b().equals(b))
        throw BadKeygen("b doesn't match");


    // Compute the key-switching data
    for (int i = 0; i < num_players; i++)
    {
        RqElement zero_q(params.FFTD(), evaluation, evaluation);
        zero_q.assign_zero();
        Encrypt_Rq_Element(ed, zero_q, player_keys[i].rc1, pk);
        add(ed_sum, ed_sum, ed);
        pk.quasi_encrypt(ezero, zero, player_keys[i].rc2);
        add(ezero_sum, ezero_sum, ezero);
    }

    global_key.secret.raise_level();
    RqElement ps(global_key.secret);
    ps.mult_by_p1(); ps.negate();

    RqElement enc0(ed_sum.c0());
    RqElement enc1(ed_sum.c1());

    add(enc0, enc0, ps);

    if (enc1 != enc_dash.c1())
        throw BadKeygen("c1 of enc_dash");
    if (enc0 != enc_dash.c0())
        throw BadKeygen("c0 of enc_dash");

    mul(enc0, enc0, global_key.secret);
    mul(enc1, enc1, global_key.secret);
    ed_sum.set(enc0, enc1, pk);
    add(ed_sum, ed_sum, ezero_sum);

    // Check key-switching data
    if (!ed_sum.c0().equals(actual_sw.c0()) ||
        !ed_sum.c1().equals(actual_sw.c1()))
      {
        throw BadKeygen("switching key doesn't match");
      }
}




/* sk will become the share of the secret key
 * pk will become the global public key
 */
void run_gen_protocol(FHE_PK& pk,FHE_SK& sk,const Player& P,int num_runs,
                      bool commit)
{
  CODE_LOCATION
  const FheParameters& params=pk.get_parameters();

#ifdef VERBOSE_KEYGEN
  double start,stop;
  /***********************
   *       Step 1        *
   ***********************/
  start=clock();
#endif

  // First compute and commit to the challenge value
  vector<unsigned int> e(P.num_players());
  vector<OctetStream> communication_e(P.num_players());
  vector<OctetStream> open_e(P.num_players());
  commit_to_challenge(e,communication_e,open_e,P,num_runs);

#ifdef VERBOSE_KEYGEN
  cout << "Done Step 1 " << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
  start=clock();
#endif

  /***********************
   *       Step 2        *
   ***********************/
  vector< vector<OctetStream> > seeds(num_runs, vector<OctetStream>(P.num_players()));
  vector< vector<OctetStream> > communication_seeds(num_runs,  vector<OctetStream>(P.num_players()));
  vector<OctetStream> open_seeds(num_runs);

  vector<PRNG> G(num_runs);
  commit_to_seeds(G,seeds,communication_seeds,open_seeds,P,num_runs);

#ifdef VERBOSE_KEYGEN
  cout << "Done Step 2 " << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
  start=clock();
#endif

  /***********************
   *       Step 2.5      *
   ***********************/

  // Generate my random data
  vector<DistKeyGenerator> keys(num_runs, {params, pk.p()});

  vector< vector<RqElement> > a(num_runs, vector<RqElement>(P.num_players(),RqElement(params.FFTD(), evaluation, evaluation)));


  for (int i = 0; i < num_runs; i++)
    {
      keys[i].gen_random_data(G[i]);
      a[i][P.my_num()] = keys[i].a;
    }

  if (commit)
    {
      // Do Commit and Open to Get a
      Commit_And_Open(a,P,num_runs);
    }
  else
    {
      transmit_data(a,P,num_runs);
    }
  for (int i=0; i<num_runs; i++)
    keys[i].sum_a(a[i]);

  a.clear();
#ifdef VERBOSE_KEYGEN
  cout << "Done Step 2.5 " << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
  start=clock();
#endif

  /***********************
   *       Step 3        *
   ***********************/
  vector< vector<RqElement> > b(num_runs, vector<RqElement>(P.num_players(),RqElement(params.FFTD(), evaluation, evaluation)));
  for (int i=0; i<num_runs; i++)
    {
      keys[i].compute_b();
      b[i][P.my_num()] = keys[i].b;
    }

#ifdef VERBOSE_KEYGEN
  cout << "Done Step 3 " << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
  start=clock();
#endif

  /***********************
   *       Step 4        *
   ***********************/
  if (commit)
    Commit_And_Open(b,P,num_runs);
  else
    transmit_data(b,P,num_runs);

#ifdef VERBOSE_KEYGEN
  cout << "Done Step 4 " << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
  start=clock();
#endif

  /***********************
   *     Step 5/6        *
   ***********************/
  vector< vector<Ciphertext> > enc_dash(num_runs, vector<Ciphertext>(P.num_players(),params));
  for (int i=0; i<num_runs; i++)
    {
      keys[i].compute_enc_dash(b[i]);
      enc_dash[i][P.my_num()] = keys[i].enc_dash;
    }

  b.clear();
#ifdef VERBOSE_KEYGEN
  cout << "Done Step 5/6 " << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
  start=clock();
#endif

  /***********************
   *       Step 7        *
   ***********************/
  if (commit)
    Commit_And_Open(enc_dash,P,num_runs);
  else
    transmit_data(enc_dash,P,num_runs);

#ifdef VERBOSE_KEYGEN
  cout << "Done Step 7 " << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
  start=clock();
#endif

  /***********************
   *    Step 8/9/10      *
   ***********************/
  vector< vector<Ciphertext> >& enc = enc_dash;
  for (int i=0; i<num_runs; i++)
    {
      keys[i].compute_enc(enc_dash[i]);
      enc[i][P.my_num()] = keys[i].enc;
   }

#ifdef VERBOSE_KEYGEN
  cout << "Done Step 8/9/10 " << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
  start=clock();
#endif

  /***********************
   *       Step 11       *
   ***********************/
  if (commit)
    Commit_And_Open(enc,P,num_runs);
  else
    transmit_data(enc,P,num_runs);

#ifdef VERBOSE_KEYGEN
  cout << "Done Step 11 " << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
  start=clock();
#endif

  /***********************
   *      Step 12        *
   ***********************/
  for (int i=0; i<num_runs; i++)
    keys[i].sum_enc(enc[i]);

#ifdef VERBOSE_KEYGEN
  cout << "Done Step 12 " << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
  start=clock();
#endif

  /***********************
   *     Step 13/14      *
   ***********************/

  int challenge=open_challenge(e,open_e,communication_e,P,num_runs);

#ifdef VERBOSE_KEYGEN
  cout << "Done Step 13/14 " << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
  start=clock();
#endif

  /***********************
   *       Step 15       *
   ***********************/
  // Dont open the challenge run
  Open(seeds,communication_seeds,open_seeds,P,num_runs,challenge);

  /********************************************************************/

  /* Now Open All Bar The Challenge Run */
  for (int i = 0; i < num_runs; i++)
    { if (i != challenge)
        {
#ifdef VERBOSE_KEYGEN
          cout << "Checking run " << i << endl;
#endif
          check_randomness(seeds[i], keys[i].enc, keys[i].pk, keys[i].enc_dash, P.num_players());
        }
    }

  // Set the key to the chosen run's output
  keys[challenge].finalize(pk, sk);

#ifdef VERBOSE_KEYGEN
  cout << "Done Step 15 " << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
  start=clock();
#endif

  P.check_broadcast();
#ifdef VERBOSE_KEYGEN
  cout << "Broadcast check all passed" << endl;

  stop=clock();
  cout << "\t\tTime = " << (stop-start)/CLOCKS_PER_SEC << " seconds " << endl;
#endif
}
