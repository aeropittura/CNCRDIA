/*
 * he/offline/producer.cpp
 *
 */

#include "he/core/p2_data.h"
#include "he/core/fft_data.h"
#include "he/offline/producer.h"
#include "he/offline/sacrificing.h"
#include "he/offline/reshare.h"
#include "he/offline/dist_decrypt.h"
#include "he/offline/simple_enc_commit.h"
#include "he/offline/simple_machine.h"
#include "support/mkpath.h"

#include "protocols/common/share.hpp"

template<class FD>
Producer<FD>::Producer(int output_thread, bool write_output) :
    n_slots(0), output_thread(output_thread), write_output(write_output),
    dir(PREP_DIR)
{
}

template<class FD>
void Producer<FD>::remove_secrets(const char* phase)
{
  auto& filename = Share<T>::secrets_filename;

  if (filename.empty())
    fprintf(stderr, "WARNING: cannot remove secrets after %s failure\n", phase);
  else
    {
      fprintf(stderr, "Removing %s because of %s failure\n", filename.c_str(),
          phase);
      unlink(filename.c_str());
    }
}

template<class FD>
void Producer<FD>::run_with_catch(const Player& P, const FHE_PK& pk,
    const Ciphertext& calpha, EncCommitBase<T, FD, S>& EC, DistDecrypt<FD>& dd,
    const T& alphai)
{
  try
    {
      run(P, pk, calpha, EC, dd, alphai);
    }
  catch (...)
    {
      remove_secrets("generation");
      throw;
    }
}

template<class FD>
int Producer<FD>::sacrifice_with_catch(const Player& P, MacCheck<T>& MC)
{
  try
    {
      return sacrifice(P, MC);
    }
  catch (...)
    {
      remove_secrets("sacrifice");
      throw;
    }
}

template <class T, class FD, class S>
TripleProducer<T, FD, S>::TripleProducer(const FD& field_d,
    int my_num, int output_thread, bool write_output, string dir) :
    Producer<FD>(output_thread, write_output),
    i(field_d.num_slots()), values{ field_d, field_d, field_d },
    macs{ field_d, field_d, field_d }, ai(values[0]), bi(values[1]),
    ci(values[2]), gam_ai(macs[0]), gam_bi(macs[1]), gam_ci(macs[2])
{
  this->dir = dir;
  for (auto& x : values)
    x.allocate_slots(field_d.get_prime());
  // extra limb for addition
  ci.allocate_slots(field_d.get_prime() << limb_size<S>());
  for (auto& x : macs)
    x.allocate_slots(field_d.get_prime() << limb_size<S>());
  if (write_output)
      this->clear_file(my_num, output_thread, false);
}

template <class FD>
TupleProducer<FD>::TupleProducer(const FD& field_d,
    int output_thread, bool write_output) :
    Producer<FD>(output_thread, write_output),
    i(field_d.num_slots()), values{ field_d, field_d },
    macs{ field_d, field_d }
{
}

template <class FD>
InverseProducer<FD>::InverseProducer(const FD& field_d,
    int my_num, int output_thread, bool write_output, bool produce_triples,
    string dir) :
    TupleProducer<FD>(field_d, output_thread, write_output), ab(field_d),
    triple_producer(field_d, 0, 0, false), produce_triples(produce_triples),
    ai(this->values[0]), bi(this->values[1]),
    gam_ai(this->macs[0]), gam_bi(this->macs[1])
{
    this->dir = dir;
    if (write_output)
        this->clear_file(my_num, output_thread, false);
}

template <class FD>
SquareProducer<FD>::SquareProducer(const FD& field_d,
    int my_num, int output_thread, bool write_output, string dir) :
    TupleProducer<FD>(field_d, output_thread, write_output),
    ai(this->values[0]), bi(this->values[1]),
    gam_ai(this->macs[0]), gam_bi(this->macs[1])
{
    this->dir = dir;
    if (write_output)
        this->clear_file(my_num, output_thread, false);
}

GfpBitProducer::GfpBitProducer(const FD& field_d,
        int my_num, int output_thread, bool write_output, bool produce_squares,
        string dir) :
        Producer<FftData>(output_thread, write_output),
        i(field_d.num_slots()), vi(field_d), gam_vi(field_d),
        square_producer(field_d, 0, 0, false), produce_squares(produce_squares)
{
    this->dir = dir;
    if (write_output)
        this->clear_file(my_num, output_thread, false);
}

template<>
Producer<FftData>* new_bit_producer(const FftData& field_d, const Player& P,
        const FHE_PK& pk, int covert,
        bool produce_squares, int thread_num, bool write_output, string dir)
{
    (void)P;
    (void)pk;
    (void)covert;
    (void)thread_num;
    (void)write_output;
    return new GfpBitProducer(field_d, P.my_num(), thread_num, write_output,
            produce_squares, dir);
}

template <class T, class FD, class S>
void TripleProducer<T, FD, S>::run(const Player& P, const FHE_PK& pk,
    const Ciphertext& calpha, EncCommitBase<T, FD, S>& EC,
    DistDecrypt<FD>& dd, const T& alphai)
{
  (void)alphai;

  const FheParameters& params=pk.get_parameters();
  map<string, Timer>& timers = this->timers;

  Ciphertext ca(params), cb(params);

  // Steps a,b,c,d
  timers["Committing"].start();
  EC.next(ai,ca);
  EC.next(bi,cb);
  timers["Committing"].stop();

  // Steps e and f
  Ciphertext cab(params),cc(params);
  timers["Multiplying"].start();
  mul(cab,ca,cb,pk);
  timers["Multiplying"].stop();
  timers["Resharing"].start();
  Reshare(ci,cc,cab,true,P,EC,pk,dd);
  timers["Resharing"].stop();

  // Step g
  Ciphertext cgam_a(params),cgam_b(params),cgam_c(params);
  timers["Multiplying"].start();
  mul(cgam_a,calpha,ca,pk);    
  mul(cgam_b,calpha,cb,pk);    
  mul(cgam_c,calpha,cc,pk);    
  timers["Multiplying"].stop();

  // Step h
  timers["Decrypting"].start();
  dd.reshare(gam_ai,cgam_a,EC);
  dd.reshare(gam_bi,cgam_b,EC);
  dd.reshare(gam_ci,cgam_c,EC);
  timers["Decrypting"].stop();

  reset();
}

template <class T>
string prep_filename(string data_type, int my_num, int thread_num,
    bool initial, string dir)
{
    stringstream file;
    file << dir;
    if (initial)
        file << "Initial-";
    file << data_type << "-" << file_completion<T>() << "-P" << my_num;
    if (thread_num)
        file << "-" << thread_num;
    return file.str();
}

template <class T>
string open_prep_file(ofstream& outf, string data_type, int my_num, int thread_num,
    bool initial, bool clear, string dir)
{
    if (mkdir_p(dir.c_str()) == -1)
        throw runtime_error("cannot create directory " + dir);
    string file = prep_filename<T>(data_type, my_num, thread_num, initial, dir);
    outf.open(file.c_str(),ios::out | ios::binary | (clear ? ios::trunc : ios::app));
    if (clear)
        file_signature<Share<T>>().output(outf);
    if (outf.fail()) { throw file_error(file); }
    return file;
}

template<class FD>
string Producer<FD>::open_file(ofstream& outf, int my_num, int thread_num,
    bool initial, bool clear)
{
    return open_prep_file<T>(outf, data_type(), my_num, thread_num, initial,
        clear, dir);
}

template <class FD>
void Producer<FD>::clear_file(int my_num, int thread_num, bool initial)
{
    ofstream outf;
    string file = this->open_file(outf, my_num, thread_num, initial, true);
    cout << "Initializing file " << file.c_str() << endl;
    outf << ""; // Write something to clear the file out
    if (outf.fail()) { throw file_error(file); }
    outf.close();
}

template <class T, class FD, class S>
int TripleProducer<T,FD,S>::sacrifice(const Player& P, MacCheck<T>& MC)
{
    this->timers["Sacrificing"].start();
    int n_triples = ai.num_slots() / 2;
    triple_checking(P, MC, n_triples, this->output_thread, *this,
            this->write_output, false, this->dir);
    this->timers["Sacrificing"].stop();
    this->n_slots = n_triples;
    return n_triples;
}

template <class FD>
int InverseProducer<FD>::sacrifice(const Player& P, MacCheck<T>& MC)
{
    this->timers["Sacrificing"].start();
    int n_triples = ai.num_slots();
    inverse_checking(P, MC, n_triples, this->output_thread, triple_producer,
            *this, this->write_output, false, this->dir);
    this->timers["Sacrificing"].stop();
    this->n_slots = n_triples;
    return n_triples;
}

template <class FD>
int SquareProducer<FD>::sacrifice(const Player& P, MacCheck<T>& MC)
{
    this->timers["Sacrificing"].start();
    int n_triples = ai.num_slots() / 2;
    square_checking(P, MC, n_triples, this->output_thread, *this,
            this->write_output, false, this->dir);
    this->timers["Sacrificing"].stop();
    this->n_slots = n_triples;
    return n_triples;
}

int GfpBitProducer::sacrifice(const Player& P, MacCheck<Gfp>& MC)
{
    timers["Sacrificing"].start();
    int n_triples = bits.size();
    bit_checking(P, MC, n_triples, output_thread, square_producer, *this,
            this->write_output, false, this->dir);
    timers["Sacrificing"].stop();
    return n_triples;
}

template<class T, class FD, class S>
void TripleProducer<T, FD, S>::get(Share<T>& a, Share<T>& b, Share<T>& c)
{
    if (i >= ai.num_slots())
        throw overflow_error("ran out of triples");
    a.set_share(ai.element(i));
    a.set_mac(gam_ai.element(i));
    b.set_share(bi.element(i));
    b.set_mac(gam_bi.element(i));
    c.set_share(ci.element(i));
    c.set_mac(gam_ci.element(i));
    i++;
}

template <class FD>
void TupleProducer<FD>::get(Share<T>& a, Share<T>& b)
{
    if (i >= values[0].num_slots())
        throw overflow_error("ran out of values");
    a.set_share(values[0].element(i));
    a.set_mac(macs[0].element(i));
    b.set_share(values[1].element(i));
    b.set_mac(macs[1].element(i));
    i++;
}

template <class FD>
void InverseProducer<FD>::get(Share<T>& a, Share<T>& b)
{
    unsigned int& i = this->i;
    while (true)
    {
        if (i >= ab.num_slots())
            throw overflow_error("ran out of triples");
        if (ab.element(i).is_zero())
            i++;
        else
            break;
    }

    TupleProducer<FD>::get(a, b);
    b /= ab.element(i - 1);
}

void GfpBitProducer::get(Share<Gfp>& a)
{
    if (i >= bits.size())
        throw overflow_error("ran out of values");
    a = bits[i];
    i++;
}

template<class T, class FD, class S>
size_t TripleProducer<T, FD, S>::report_size(ReportType type)
{
    return ai.report_size(type) + bi.report_size(type) + ci.report_size(type)
            + gam_ai.report_size(type) + gam_bi.report_size(type)
            + gam_ci.report_size(type);
}

template <class FD>
void InverseProducer<FD>::run(const Player& P, const FHE_PK& pk, const Ciphertext& calpha,
        EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd, const T& alphai)
{
    (void)P;
    (void)alphai;

    const FheParameters& params = pk.get_parameters();

    // Steps a,b,c,d
    Ciphertext ca(pk), cb(pk);
    EC.next(ai,ca);
    EC.next(bi,cb);

    // Steps e and f
    Ciphertext cab(params),cc(params);
    mul(cab,ca,cb,pk);
    dd.run(cab, true);
    ab = dd.mf;

    if (produce_triples)
        triple_producer.run(P, pk, calpha, EC, dd, alphai);

    this->compute_macs(pk, calpha, EC, dd, ca, cb);
}

template <class FD>
void TupleProducer<FD>::compute_macs(const FHE_PK& pk, const Ciphertext& calpha,
        EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd, Ciphertext& ca,
        Ciphertext & cb)
{
    const FheParameters& params = pk.get_parameters();

   // Step g
    Ciphertext cgam_a(params),cgam_b(params);
    mul(cgam_a,calpha,ca,pk);
    mul(cgam_b,calpha,cb,pk);

    // Step h
    Ciphertext dummy(params);
    dd.reshare(macs[0], cgam_a, EC);
    dd.reshare(macs[1], cgam_b, EC);

    i = 0;
}

template<class FD>
void SquareProducer<FD>::run(const Player& P, const FHE_PK& pk,
        const Ciphertext& calpha, EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd,
        const T& alphai)
{
    (void)alphai;

    // Steps a,b
    Ciphertext ca(pk);
    EC.next(ai,ca);

    // Steps c and d
    Ciphertext caa(pk),cb(pk);
    mul(caa,ca,ca,pk);
    Reshare(bi,cb,caa,true,P,EC,pk,dd);

    this->compute_macs(pk, calpha, EC, dd, ca, cb);
}

void GfpBitProducer::run(const Player& P, const FHE_PK& pk,
        const Ciphertext& calpha, EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd,
        const Gfp& alphai)
{
    const FheParameters& params = pk.get_parameters();

    Gfp one;
    one.assign_one();

    // Steps a,b
    Plaintext_<FftData> ai(dd.f.get_field());
    Ciphertext ca(pk);
    EC.next(ai, ca);

    // Steps c and d
    Ciphertext caa(params);
    mul(caa, ca, ca, pk);
    Plaintext_<FftData>& s = dd.run(caa, true);

    // Step e, f and g
    vector<int> marks(s.num_slots());
    for (unsigned int i = 0; i < s.num_slots(); i++)
    {
        if (s.element(i).is_zero())
        {
            s.set_element(i, one);
            marks[i] = 1;
        }
        else
        {
            marks[i] = 0;
            s.set_element(i, s.element(i).square_root().invert());
        }
    }
    Ciphertext cv(params);
    mul(cv, s, ca);

    // Steps h
    Ciphertext cgam_v(params);
    mul(cgam_v, calpha, cv, pk);

    // Step i
    Ciphertext dummy(params);
    dd.reshare(vi, cv, EC);
    dd.reshare(gam_vi, cgam_v, EC);

    // Step j and k
    Share<Gfp> a;
    Gfp two_inverse, zero;
    two_inverse = Bigint((dd.f.get_field().get_prime() + 1) / 2);
    zero.assign_zero();
    auto shared_one = Share<Gfp>::constant(1, P.my_num(), alphai);
    bits.clear();
    for (unsigned int i = 0; i < vi.num_slots(); i++)
    {
        if (marks[i] == 0)
        {
            a.set_share(vi.element(i));
            a.set_mac(gam_vi.element(i));
            // Form (1/2)*a+1
            a += shared_one;
            a.mul(a, two_inverse);
            bits.push_back(a);
        }
    }

    if (produce_squares)
        square_producer.run(P, pk, calpha, EC, dd, alphai);

    i = 0;
    this->n_slots = bits.size();
}

Gf2nBitProducer::Gf2nBitProducer(const Player& P, const FHE_PK& pk, int covert,
        int thread_num, bool write_output, string dir) :
        Producer<P2Data>(thread_num, write_output),
        write_output(write_output), ECB(P, pk, covert, Bits)
{
    if (covert == 0)
        throw NotImplemented();
    this->dir = dir;
    if (write_output)
        open_prep_file<Gf2nShort>(outf, data_type(), P.my_num(), thread_num,
                false, true, dir);
}

template<>
Producer<P2Data>* new_bit_producer(const P2Data& field_d, const Player& P,
        const FHE_PK& pk, int covert,
        bool produce_squares, int thread_num, bool write_output, string dir)
{
    (void)field_d;
    (void)produce_squares;
    return new Gf2nBitProducer(P, pk, covert, thread_num, write_output, dir);
}

void Gf2nBitProducer::run(const Player& P, const FHE_PK& pk,
        const Ciphertext& calpha, EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd,
        const T& alphai)
{
    (void)P;
    (void)alphai;

    const FheParameters& params = pk.get_parameters();
    const P2Data& field_d = dd.f.get_field();

    this->n_slots = field_d.num_slots();

    Plaintext<Gf2nShort,P2Data,int> ai(field_d);
    Ciphertext ca(pk);

    // Steps a,b,c,d
    ECB.next(ai,ca);

    // Step g
    Ciphertext cgam_a(params);
    mul(cgam_a,calpha,ca,pk);

    // Step h
    Plaintext<Gf2nShort,P2Data,int> gam_ai(field_d);
    Ciphertext dummy(params);
    dd.reshare(gam_ai, cgam_a, EC);

    // Step i
    Share<Gf2nShort> a;
    for (unsigned int i=0; i<ai.num_slots(); i++)
    {
        a.set_share(ai.element(i));
        a.set_mac(gam_ai.element(i));
        if (write_output)
            a.output(outf, false);
    }
}

template <class FD>
InputProducer<FD>::InputProducer(const Player& P, int thread_num,
        bool write_output, string dir) :
        Producer<FD>(thread_num, write_output),
        P(P), write_output(write_output)
{
    this->dir = dir;

    // Cannot form vectors of streams...
    outf = new ofstream[P.num_players()];

    // Open files for writing out data
    if (write_output)
        for (int j = 0; j < P.num_players(); j++)
        {
            stringstream file;
            file << dir << "Inputs-" << file_completion<T>() << "-P"
                    << P.my_num() << "-" << j;
            if (thread_num)
                file << "-" << thread_num;
            outf[j].open(file.str().c_str(), ios::out | ios::binary);
            file_signature<Share<T>>().output(outf[j]);
            if (outf[j].fail())
            {
                throw file_error(file.str());
            }
        }
}

template<class FD>
InputProducer<FD>::~InputProducer()
{
    if (write_output)
        for (int j=0; j<P.num_players(); j++)
            outf[j].close();
    delete[] outf;
}

template<class FD>
void InputProducer<FD>::run(const Player& P, const FHE_PK& pk,
        const Ciphertext& calpha, EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd,
        const T& alphai, int player)
{
    CODE_LOCATION
    (void)alphai;

    const FheParameters& params=pk.get_parameters();

    PRNG G;
    G.re_seed();

    Ciphertext gama(params),dummyc(params),ca(params);
    const FD& field_d = dd.f.get_field();
    Plaintext<T,FD,S> a(field_d),ai(field_d),gai(field_d);

    this->n_slots = field_d.num_slots();

    Share<T> sh;

    inputs.resize(P.num_players());

    int min, max;
    if (player < 0)
    {
        min = 0;
        max = P.num_players();
    }
    else
    {
        min = player;
        max = player + 1;
    }

    map<string, Timer> timers;
    assert(EC.machine);
    SimpleEncCommit_<FD> personal_ec(P, pk, field_d, timers, *EC.machine, 0);
    OctetStream ciphertexts, cleartexts;
    personal_ec.get_proof().set_session_id(P);

    for (int j = min; j < max; j++)
    {
        AddableVector<Ciphertext> C;
        vector<Plaintext_<FD>> m(personal_ec.proof.U, field_d);
        if (j == P.my_num())
        {
            for (auto& x : m)
                x.randomize(G);
            personal_ec.generate_proof(C, m, ciphertexts, cleartexts);
            P.send_all(ciphertexts);
            P.send_all(cleartexts);
        }
        else
        {
            P.receive_player(j, ciphertexts);
            P.receive_player(j, cleartexts);
            C.resize(personal_ec.machine->sec, pk.get_parameters());
            Verifier<FD>(personal_ec.proof, field_d).nizk_pok(C, ciphertexts,
                    cleartexts, pk, j);
        }

        inputs[j].clear();

        for (size_t i = 0; i < C.size(); i++)
        {
            auto& ca = C.at(i);
            auto& a = m.at(i);

            // Reshare the aj values
            dd.reshare(ai, ca, EC);

            // Generate encrypted MAC values
            mul(gama, calpha, ca, pk);

            // Get shares on the MACs
            dd.reshare(gai, gama, EC);

            for (unsigned int i = 0; i < ai.num_slots(); i++)
            {
                sh.set_share(ai.element(i));
                sh.set_mac(gai.element(i));
                if (write_output)
                {
                    sh.output(outf[j], false);
                    if (j == P.my_num())
                    {
                        a.element(i).output(outf[j], false);
                    }
                }
                else
                {
                    inputs[j].push_back({sh, {}});
                    if (j == P.my_num())
                        inputs[j].back().value = a.element(i);
                }
            }
        }
    }
}

template<class FD>
int InputProducer<FD>::sacrifice(const Player& P, MacCheck<T>& MC)
{
    // nothing to do
    (void)P;
    (void)MC;
    return this->num_slots();
}

int Gf2nBitProducer::sacrifice(const Player& P, MacCheck<T>& MC)
{
    // nothing to do
    (void)P;
    (void)MC;
    return this->num_slots();
}

template<class FD>
string InputProducer<FD>::open_file(ofstream& outf, int my_num,
        int thread_num, bool initial, bool clear)
{
    (void)outf;
    (void)my_num;
    (void)thread_num;
    (void)initial;
    (void)clear;
    return "";
}

template<class FD>
void InputProducer<FD>::clear_file(int my_num, int thread_num,
        bool initial)
{
    (void)my_num;
    (void)thread_num;
    (void)initial;
}


template class TripleProducer<Gfp, FftData, Bigint>;
template class TripleProducer<Gf2nShort, P2Data, int>;
template class Producer<FftData>;
template class Producer<P2Data>;
template class TupleProducer<FftData>;
template class TupleProducer<P2Data>;
template class SquareProducer<FftData>;
template class SquareProducer<P2Data>;
template class InputProducer<FftData>;
template class InputProducer<P2Data>;
template class InverseProducer<FftData>;
template class InverseProducer<P2Data>;

template string open_prep_file<Gfp>(ofstream& outf, string data_type,
        int my_num, int thread_num, bool initial, bool clear, string dir);
template string open_prep_file<Gf2nShort>(ofstream& outf, string data_type,
        int my_num, int thread_num, bool initial, bool clear, string dir);
