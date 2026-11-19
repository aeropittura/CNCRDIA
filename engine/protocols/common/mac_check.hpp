#pragma once

#include "protocols/common/mac_check.h"
#include "support/subroutines.h"
#include "support/exceptions.h"

#include "support/random.h"
#include "support/time_func.h"
#include "support/int.h"
#include "support/benchmarking.h"
#include "support/bundle.h"

#include "ot/mascot_mac_key.h"
#include "boolean/tinier_secret.h"
#include "protocols/spdz/mascot_prep.h"

#include <algorithm>

#include "protocols/common/mac_check_base.hpp"
#include "protocols/common/mac_key.hpp"
#include "ot/mascot_mac_key.hpp"

template<class T>
const char* TreeSum<T>::opener_timer_names[] = {
        "sending",
        "receiving and adding",
        "broadcasting",
        "receiving summed values",
        "random seed",
        "commit and open",
        "wait for summer thread",
        "receiving",
        "summing",
        "waiting for select()"
};

template<class U>
void TreeMacCheck<U>::setup(Player& P)
{
  assert(coordinator == 0);
  coordinator = new Coordinator(P.N, U::type_string() + to_string(U::clear::length()));
  U::PrepType::MacCheck::coordinator = coordinator;
  U::PrepCheckType::MacCheck::coordinator = coordinator;
  U::BitPrepType::MacCheck::coordinator = coordinator;
  U::BitPrepType::PrepCheckType::MacCheck::coordinator = coordinator;
  U::InputCheckType::MacCheck::coordinator = coordinator;
}

template<class U>
void TreeMacCheck<U>::teardown()
{
  if (coordinator)
    delete coordinator;
  coordinator = 0;
  U::PrepType::MacCheck::coordinator = 0;
  U::PrepCheckType::MacCheck::coordinator = 0;
  U::BitPrepType::MacCheck::coordinator = 0;
  U::BitPrepType::PrepCheckType::MacCheck::coordinator = 0;
  U::InputCheckType::MacCheck::coordinator = 0;
}

template<class U>
MacCheck_<U>::MacCheck_(const typename U::MacKeyType::Scalar& ai, int opening_sum,
    int max_broadcast, int send_player) :
    TreeMacCheck<U>(ai, opening_sum, max_broadcast, send_player)
{
}

template<class U>
TreeMacCheck<U>::TreeMacCheck(const typename U::MacKeyType::Scalar& ai, int opening_sum,
    int max_broadcast, int send_player) :
    TreeSum<T>(opening_sum, max_broadcast, send_player)
{
  popen_count=0;
  this->alphai=ai;
}

template<class T>
TreeMacCheck<T>::~TreeMacCheck()
{
#ifndef NO_SECURITY_CHECK
  if (waiting_for_check() > 0)
    {
      cerr << endl << "SECURITY BUG: insufficient checking" << endl;
      terminate();
    }
#endif
}

template<class U>
void TreeMacCheck<U>::initialize_open(const Player&, int n)
{
  if (n > waiting_for_check())
    {
      macs.reserve(macs.size() + n);
      vals.reserve(vals.size() + n);
    }

  this->secrets.clear();
  this->values.clear();
  this->secrets.reserve(n);
  this->values.reserve(n);
}

template<class U>
void TreeMacCheck<U>::prepare_open(const U& secret, int)
{
  assert(U::MacType::invertible);
  this->values.push_back(secret.get_share());
  macs.push_back(secret.get_mac());
}

template<class U>
void TreeMacCheck<U>::exchange(const Player& P)
{
  this->run(this->values, P);

  this->values_opened += this->values.size();

  popen_count += this->values.size();

  if (OnlineOptions::singleton.has_option("always_check"))
    Check(P);
  else
    check_if_needed(P);
}

template<class U>
void MacCheck_<U>::start_exchange(const Player& P)
{
    this->exchange(P);
}

template<class U>
void MacCheck_<U>::stop_exchange(const Player&)
{
}


template<class U>
void TreeMacCheck<U>::add_to_values(vector<T>& values)
{
  vals.insert(vals.end(), values.begin(), values.end());
}


template<class T>
void TreeMacCheck<T>::check_if_needed(const Player& P)
{
  if (waiting_for_check() >= POPEN_MAX)
    Check(P);
}


template <class U>
void TreeMacCheck<U>::AddToCheck(const U& share, const T& value, const Player& P)
{
  macs.push_back(share.get_mac());
  vals.push_back(value);
  popen_count++;
  check_if_needed(P);
}



template<class U>
void mac_fail_remove(const Player& P)
{
  string filename;
  if (not U::secrets_filename.empty())
    filename = U::secrets_filename;
  else if (U::needs_ot)
    {
      if (not U::PrepType::clear::Scalar::length())
        U::PrepType::clear::Scalar::template init<typename U::clear>(false);
      filename = get_ot_secrets_filename<typename U::PrepType>(P);
    }
  else
    filename = U::LivePrep::get_full_secrets_filename(P);
  cerr << "Removing " << filename << " because of MAC check failure" << endl;
  assert(unlink(filename.c_str()) == 0);
  throw MacFail();
}

template<class U>
void MacCheck_<U>::Check(const Player& P)
{
  assert(U::MacType::invertible);
  check_field_size<typename U::MacType>();

  if (this->waiting_for_check() == 0)
    return;

  //cerr << "In MAC Check : " << popen_count << endl;

  CODE_LOCATION
  auto& vals = this->vals;
  auto& macs = this->macs;
  auto& popen_count = this->popen_count;
  assert(int(macs.size()) <= popen_count);
  assert(this->coordinator);
  bool debug = OnlineOptions::singleton.has_option("debug_mac");

  if (OnlineOptions::singleton.has_option("mac_fail"))
    vals.at(0) += P.my_num();

  if (popen_count < 10 and debug)
    {
      // no random combination with few values
      vector<typename U::MacType> deltas;
      Bundle<OctetStream> bundle(P);
      for (int i = 0; i < popen_count; i++)
        {
          deltas.push_back(vals[i] * this->alphai - macs[i]);
          if (debug)
            cout << "MAC check value=" << vals[i] << " key=" << this->alphai
                << " MAC=" << macs[i] << endl;
          deltas.back().pack(bundle.mine);
        }
      this->timers[COMMIT].start();
      commit_and_open_(bundle, P, *this->coordinator);
      this->timers[COMMIT].stop();
      reset();
      for (auto& delta : deltas)
        {
          for (auto& os : bundle)
            if (&os != &bundle.mine)
              delta += os.get<typename U::MacType>();
          if (delta != 0)
            mac_fail_remove<U>(P);
        }
    }
  else
    {
      // check random combination
      Octet seed[SEED_SIZE];
      this->timers[SEED].start();
      create_random_seed(seed,P,SEED_SIZE);
      this->timers[SEED].stop();
      PRNG G;
      G.SetSeed(seed);

      U sj;
      typename U::MacType a,gami,temp;
      typename U::MacType::Scalar h;
      vector<typename U::MacType> tau(P.num_players());
      for (int i=0; i<popen_count; i++)
        {
          h.almost_randomize(G);
          temp = vals[i] * h;
          a = (a + temp);

          temp = h * macs[i];
          gami = (gami + temp);
        }

      temp = this->alphai * a;
      tau[P.my_num()] = (gami - temp);

      //cerr << "\tCommit and Open" << endl;
      this->timers[COMMIT].start();
      Commit_And_Open(tau, P, *this->coordinator);
      this->timers[COMMIT].stop();

      //cerr << "\tFinal Check" << endl;

      typename U::MacType t;
      for (int i=0; i<P.num_players(); i++)
        { t += tau[i]; }
      reset();
      if (t != 0)
        mac_fail_remove<U>(P);
    }

  this->coordinator->finished();
}

template<class U>
void MacCheck_<U>::reset()
{
  auto& vals = this->vals;
  auto& macs = this->macs;
  auto& popen_count = this->popen_count;

  vals.erase(vals.begin(), vals.begin() + popen_count);
  macs.erase(macs.begin(), macs.begin() + popen_count);

  this->popen_count=0;
}

template<class T, class U, class V, class W>
MacCheckZ2k<T, U, V, W>::MacCheckZ2k(const T& ai, int opening_sum, int max_broadcast, int send_player) :
    TreeMacCheck<W>(ai, opening_sum, max_broadcast, send_player), prep(0)
{
}

template<class T, class U, class V, class W>
MacCheckZ2k<T, U, V, W>::MacCheckZ2k(const T& ai, Names& nms,
        int thread_num) : MacCheckZ2k(ai)
{
    (void) nms, (void) thread_num;
}

template<class T, class U, class V, class W>
void MacCheckZ2k<T, U, V, W>::prepare_open(const W& secret, int)
{
  prepare_open_no_mask(secret + (get_random_element() << W::clear::N_BITS));
}

template<class T, class U, class V, class W>
void MacCheckZ2k<T, U, V, W>::prepare_open_no_mask(const W& secret)
{
  this->values.push_back(secret.get_share());
  this->macs.push_back(secret.get_mac());
}

template<class T, class U, class V, class W>
W MacCheckZ2k<T, U, V, W>::get_random_element() {
  if (random_elements.size() > 0)
    {
      W res = random_elements.back();
      random_elements.pop_back();
      return res;
    }
  else
    {
      if (prep)
        return prep->get_random_for_open();
      else
      {
        insecure("random dummy");
        return {};
      }
    }
}

template<class T, class U, class V, class W>
void MacCheckZ2k<T, U, V, W>::set_random_element(const W& random_element) {
  random_elements.push_back(random_element);
}

template<class T, class U, class V, class W>
void MacCheckZ2k<T, U, V, W>::set_prep(Preprocessing<W>& prep)
{
  this->prep = &prep;
}

template<class T, class U, class V, class W>
void MacCheckZ2k<T, U, V, W>::Check(const Player& P)
{
  if (this->waiting_for_check() == 0)
    return;

  if (OnlineOptions::singleton.has_option("mac_fail"))
    this->vals.at(0) += P.my_num();

  CODE_LOCATION
#ifdef DEBUG_MAC
  cout << "Checking " << shares[0] << " " << this->vals[0] << " " << this->macs[0] << endl;
#endif

  Octet seed[SEED_SIZE];
  create_random_seed(seed,P,SEED_SIZE);
  PRNG G;
  G.SetSeed(seed);

  T y, mj;
  y.assign_zero();
  mj.assign_zero();
  for (int i = 0; i < this->popen_count; ++i)
  {
    U temp_chi;
    temp_chi.randomize(G);
    T xi = this->vals[i];
    y += xi * temp_chi;
    T mji = this->macs[i];
    mj += temp_chi * mji;
  }

  T zj = mj - this->alphai * y;
  vector<T> zjs(P.num_players());
  zjs[P.my_num()] = zj;

  assert(this->coordinator);
  Commit_And_Open(zjs, P, *this->coordinator);

  T zj_sum;
  zj_sum.assign_zero();
  for (int i = 0; i < P.num_players(); ++i)
    zj_sum += zjs[i];

  this->vals.erase(this->vals.begin(), this->vals.begin() + this->popen_count);
  this->macs.erase(this->macs.begin(), this->macs.begin() + this->popen_count);
  this->popen_count=0;
  if (!zj_sum.is_zero())
    mac_fail_remove<W>(P);

  this->coordinator->finished();
}



template<class T>
DirectMacCheck<T>::DirectMacCheck(const typename T::MacKeyType::Scalar& ai,
    Names&, int) :
    DirectMacCheck<T>(ai)
{
}

template<class T>
DirectMacCheck<T>::DirectMacCheck(const typename T::MacKeyType::Scalar& ai) :
    TreeMacCheck<T>(ai), MacCheck_<T>(ai)
{
  open_counter = 0;
}


template<class T>
DirectMacCheck<T>::~DirectMacCheck() {
  cerr << T::type_string() << " open counter: " << open_counter << endl;
}

template<class T>
void direct_add_openings(vector<T>& values, const PlayerBase& P, vector<OctetStream>& os)
{
  size_t n = P.num_players();
  size_t me = P.my_num();
  assert(os.size() == n);
  for (auto& value : values)
    for (size_t j = 0; j < n; j++)
      if (j != me)
        value += os[j].get<T>();
}

template<class T>
void DirectMacCheck<T>::pre_exchange(const Player& P)
{
  oss.resize(P.num_players());
  oss[P.my_num()].reset_write_head();
  oss[P.my_num()].reserve(this->values.size() * T::OpenType::size());

  for (auto& x : this->values)
    x.pack(oss[P.my_num()]);
  oss[P.my_num()].append(0);
}


template<class T>
void DirectMacCheck<T>::exchange(const Player& P)
{
  CODE_LOCATION
  pre_exchange(P);
  P.unchecked_broadcast(oss);

  open_counter++;

  direct_add_openings<OpenType>(this->values, P, oss);

  this->add_to_values(this->values);

  this->popen_count += this->values.size();
  this->check_if_needed(P);
}

template<class T>
void DirectMacCheck<T>::initialize_open(const Player& P, int n)
{
  Super::initialize_open(P, n);
}

template<class T>
void DirectMacCheck<T>::prepare_open(const T& secret, int)
{
  assert(T::MacType::invertible);
  this->values.push_back(secret.get_share());
  this->macs.push_back(secret.get_mac());
}
