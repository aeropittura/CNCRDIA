#pragma once

#include "vm/program.h"
#include "vm/online_thread.h"
#include "support/time_func.h"
#include "vm/data_files.h"
#include "vm/machine.h"
#include "vm/processor.h"
#include "net/crypto_player.h"
#include "protocols/common/shuffle_sacrifice.h"
#include "protocols/common/limited_prep.h"
#include "he/core/fft.h"
#include "ot/ot_extension_with_matrix.h"

#include "vm/processor.hpp"
#include "vm/instruction.hpp"
#include "vm/input.hpp"
#include "protocols/common/limited_prep.hpp"
#include "protocols/replicated/malrep_ring_prep.hpp"
#include "boolean/bit_adder.hpp"

#include <iostream>
#include <fstream>
#include <pthread.h>
using namespace std;


template<class Sint, class Sgf2n>
template<class T>
void thread_info<Sint, Sgf2n>::print_usage(ostream &o,
        const vector<T>& regs, const char* name)
{
  ::print_usage(o, name, regs.capacity());
}

template<class Sint, class Sgf2n>
void thread_info<Sint, Sgf2n>::sub_main_func()
{
  Bigint::initialize_thread();

  auto tinfo = this;
  Machine<Sint, Sgf2n>& machine=*(tinfo->machine);
  vector<Program>& progs                = machine.progs;

  int num=tinfo->thread_num;
  BaseMachine::s().thread_num = num;

  auto& queues = machine.queues[num];
  auto& opts = machine.opts;
  queues->next();
  ThreadQueue::thread_queue = queues;

#ifdef DEBUG_THREADS
  fprintf(stderr, "\tI am in thread %d\n",num);
#endif
  Player* player;
  string id = "thread" + to_string(num);
  if (machine.use_encryption)
    {
#ifdef VERBOSE_OPTIONS
      cerr << "Using encrypted single-threaded communication" << endl;
#endif
      player = new CryptoPlayer(*(tinfo->nms), id);
    }
  else if (!opts.receive_threads or opts.direct)
    {
#ifdef VERBOSE_OPTIONS
      cerr << "Using single-threaded receiving" << endl;
#endif
      player = new PlainPlayer(*(tinfo->nms), id);
    }
  else
    {
#ifdef VERBOSE_OPTIONS
      cerr << "Using player-specific threads for receiving" << endl;
#endif
      player = new ThreadPlayer(*(tinfo->nms), id);
    }
  Player& P = *player;
#ifdef DEBUG_THREADS
  fprintf(stderr, "\tSet up player in thread %d\n",num);
#endif

  typename Sgf2n::MacCheck* MC2;
  typename Sint::MacCheck*  MCp;

  if (opts.direct)
    {
#ifdef VERBOSE_OPTIONS
      cerr << "Using direct communication." << endl;
#endif
      MC2 = new typename Sgf2n::DirectOpener(*(tinfo->alpha2i));
      MCp = new typename Sint::DirectOpener(*(tinfo->alphapi));
    }
  else
    {
#ifdef VERBOSE_OPTIONS
      cerr << "Using indirect communication." << endl;
#endif
      MC2 = new typename Sgf2n::MacCheck(*(tinfo->alpha2i), opts.opening_sum, opts.max_broadcast);
      MCp = new typename Sint::MacCheck(*(tinfo->alphapi), opts.opening_sum, opts.max_broadcast);
    }

  // Allocate memory for first program before starting the clock
  processor = new Processor<Sint, Sgf2n>(tinfo->thread_num,P,*MC2,*MCp,machine,progs.at(thread_num > 0));
  auto& Proc = *processor;

  // don't count communication for initialization
  P.reset_statistics();

  bool flag=true;
  int program=-3; 
  // int exec=0;

  typedef typename Sint::BitType::part_type BT;

  // synchronize
#ifdef DEBUG_THREADS
  cerr << "Locking for sync of thread " << num << endl;
#endif
  queues->finished({});

  DataPositions actual_usage(P.num_players());
  Timer thread_timer(CLOCK_THREAD_CPUTIME_ID);
  thread_timer.start();
  queues->start_timer();

  while (flag)
    { // Wait until I have a program to run
      ThreadJob job = queues->next();
      program = job.prognum;
#ifdef DEBUG_THREADS
      printf("\tRunning program %d/job %d in thread %d\n", program, job.type,
          num);
#endif

      if (program==-1)
        { flag=false;
#ifdef DEBUG_THREADS
          fprintf(stderr, "\tThread %d terminating\n",num);
#endif
        }
      else if (job.type == BITADD_JOB)
        {
          auto& party = GC::ShareThread<typename Sint::BitType>::s();
          if (job.arg < 0)
            {
              SubProcessor<BT> bit_processor(party.MC->get_part_opener(),
                  Proc.Procp.bit_prep, P);
              BitAdder().add(*(vector<vector<BT>>*) job.output,
                  *(vector<vector<vector<BT>>>*) job.input, job.begin, job.end,
                  bit_processor, job.length, -1, job.supply);
            }
          else
            {
              // too late for preprocessing for security reasons
              assert(job.supply);
              LimitedPrep<BT> limited_prep;
              SubProcessor<BT> bit_processor(party.MC->get_part_opener(),
                  limited_prep, P);
              BitAdder().add(*(vector<vector<BT>>*) job.output,
                  *(vector<vector<vector<BT>>>*) job.input, job.begin, job.end,
                  bit_processor, job.length, -1, job.supply);
            }
          queues->finished(job);
        }
      else if (job.type == DABIT_JOB)
        {
          dynamic_cast<RingPrep<Sint>&>(Proc.DataF.data_fp).template
                  buffer_dabits_without_check<0>(
              *(vector<Dabit<Sint>>*) job.output, job.begin, job.end,
              Proc.Procp.bit_prep);
          queues->finished(job);
        }
      else if (job.type == MULT_JOB)
        {
          Proc.Procp.protocol.multiply(*(PointerVector<Sint>*) job.output,
              *(vector<pair<Sint, Sint>>*) job.input, job.begin, job.end,
              Proc.Procp);
          queues->finished(job);
        }
      else if (job.type == EDABIT_JOB)
        {
          dynamic_cast<RingPrep<Sint>&>(Proc.DataF.data_fp).template
                  buffer_edabits_without_check<0>(
              job.length, *(vector<Sint>*) job.output,
              *(vector<vector<BT>>*) job.output2, job.begin, job.end);
          queues->finished(job);
        }
      else if (job.type == PERSONAL_JOB)
        {
          auto &party = GC::ShareThread<typename Sint::BitType>::s();
          SubProcessor<BT> bit_processor(party.MC->get_part_opener(),
              Proc.Procp.bit_prep, P);
          dynamic_cast<RingPrep<Sint>&>(Proc.DataF.data_fp).template
                  buffer_personal_edabits_without_check<0>(
              job.length, *(vector<Sint>*) job.output,
              *(vector<vector<BT>>*) job.output2, bit_processor,
              job.arg, job.begin, job.end);
          queues->finished(job);
        }
      else if (job.type == SANITIZE_JOB)
        {
          dynamic_cast<RingPrep<Sint>&>(Proc.DataF.data_fp).template
                  sanitize<0>(
              *(vector<Edabit<Sint>>*) job.output, job.length, job.arg,
              job.begin, job.end);
          queues->finished(job);
        }
      else if (job.type == EDABIT_SACRIFICE_JOB)
        {
          Sint::LivePrep::edabit_sacrifice_buckets(
              *(vector<Edabit<Sint>>*) job.output, job.length, job.prognum,
              job.arg, Proc.Procp,
              job.begin, job.end, job.supply);
#ifdef DEBUG_THREADS
          printf("\tSignalling I have finished with job %d in thread %d\n",
              job.type, num);
#endif
          queues->finished(job);
        }
      else if (job.type == PERSONAL_TRIPLE_JOB)
        {
          auto &party = GC::ShareThread<typename Sint::BitType>::s();
          SubProcessor<BT> bit_processor(party.MC->get_part_opener(),
              *Proc.Procp.personal_bit_preps.at(job.arg), P);
          Proc.Procp.personal_bit_preps.at(job.arg)->buffer_personal_triples(
              *(vector<array<BT, 3>>*) job.output, job.begin, job.end);
          queues->finished(job);
        }
      else if (job.type == TRIPLE_SACRIFICE_JOB)
        {
          typedef typename Sint::BitType B;
          auto &party = GC::ShareThread<B>::s();
          TripleShuffleSacrifice<B>(job.arg).triple_sacrifice(
              *(vector<array<B, 3>>*) job.output,
              *(vector<array<B, 3>>*) job.input, *party.P, *party.MC, job.begin,
              job.end);
          queues->finished(job);
        }
      else if (job.type == CHECK_JOB)
        {
          Proc.check();
          queues->finished(job);
        }
      else if (job.type == FFT_JOB)
        {
          for (int i = job.begin; i < job.end; i++)
            fft_iter2_body(*(vector<modp>*) job.output,
                *(vector<modp>*) job.input, i, job.length,
                *(ZpData*) job.supply);
          queues->finished(job);
        }
      else if (job.type == CIPHER_PLAIN_MULT_JOB)
        {
          cipher_plain_mult(job, Sint::triple_matmul);
          queues->finished(job);
        }
      else if (job.type == MATRX_RAND_MULT_JOB)
        {
          matrix_random_mult(job, Sint::triple_matmul);
          queues->finished(job);
        }
      else if (job.type == SHUFFLE_JOB)
        {
          assert(job.output);
          assert(job.output2);
          assert(job.supply);
          ((typename Sint::Protocol::Shuffler*) job.output)->shuffle_job(
              *(StackedVector<Sint>*) job.output2,
              *(const vector<ShuffleTuple<typename Sint::Protocol::Shuffler::ShuffleType>>*)
              job.supply, job.begin,
              job.end, P, &machine.queues);
          queues->finished(job);
        }
      else if (job.type == OTLSB_JOB)
        {
          assert(job.output);
          assert(job.input);
          Proc.Procp.protocol.get_ot_extension().bit_extension(
                  *(OtExtensionWithMatrix::OtBits*) job.output,
                  *(const BitVector*) job.input, job.begin, job.end);
          queues->finished(job);
        }
      else if (job.type == TAPE_JOB)
        { // RUN PROGRAM
#ifdef DEBUG_THREADS
          printf("\tClient %d about to run %d\n",num,program);
#endif
          queues->start_online(P, Proc.prep_time());
          Proc.reset(progs[program], job.arg);

          // Bits, Triples, Squares, and Inverses skipping
          Proc.DataF.seekg(job.pos);
          // reset for actual usage
          Proc.DataF.reset_usage();
             
          //printf("\tExecuting program");
          // Execute the program
          BaseMachine::program = &progs[program];
          progs[program].execute(Proc);

          // make sure values used in other threads are safe
          Proc.check();

          // prevent mangled output
          cout.flush();

          actual_usage.increase(Proc.DataF.get_usage());

         if (progs[program].usage_unknown())
           { // communicate file positions to main thread
             job.pos.increase(Proc.DataF.get_usage());
           }

#ifdef DEBUG_THREADS
          printf("\tSignalling I have finished with program %d"
              "in thread %d\n", program, num);
#endif
          queues->stop_online(P, Proc.prep_time());
          queues->finished(job, {P.total_communication(), Proc.statistics});
       }  
      else
        throw runtime_error("unknown job type " + to_string(job.type));
    }

  // final check
  queues->start_online(P, Proc.prep_time());
  Proc.check();
  queues->stop_online(P, Proc.prep_time());

  if (machine.opts.file_prep_per_thread)
    Proc.DataF.prune();

  queues->next();
  queues->stop_timer(P);

#ifdef VERBOSE
  if (MC2->number() + MCp->number() > 0)
    cerr << num << " : MAC Checking" << endl;
  if (MC2->number())
    cerr << "\tMC2.number=" << MC2->number() << endl;
  if (MCp->number())
    cerr << "\tMCp.number=" << MCp->number() << endl;

  cerr << "Thread " << num << " timer: " << thread_timer.elapsed() << endl;
  cerr << "Thread " << num << " wait timer: " << wait_timer.elapsed() << endl;

  cerr << "Register usage: ";
  print_usage(cerr, Proc.Procp.get_s(), "sint");
  print_usage(cerr, Proc.Procp.get_c(), "cint");
  print_usage(cerr, Proc.Proc2.get_s(), "sgf2n");
  print_usage(cerr, Proc.Proc2.get_c(), "cgf2n");
  print_usage(cerr, Proc.procb.S, "sbits");
  print_usage(cerr, Proc.procb.C, "cbits");
  print_usage(cerr, Proc.get_ci(), "regint");
  cerr << endl;
#endif

  if (num == 0 and OnlineOptions::singleton.verbose
      and machine.queues.size() > 1)
    {
      cerr << "Main thread communication:" << endl;
      P.total_communication().print();
    }

  // wind down thread by thread

  if (not Proc.statistics.empty() and
      OnlineOptions::singleton.verbose)
    {
      cerr << "Thread " << thread_num << endl;
      Proc.statistics.print();
    }

  assert(Proc.share_thread.protocol);
  queues->timers["random"] = Proc.Procp.protocol.randomness_time()
      + Proc.Proc2.protocol.randomness_time()
      + Proc.share_thread.protocol->randomness_time();

  NamedStatistics statistics;
  statistics["integer multiplications"] = Proc.Procp.protocol.counter;
  statistics["integer multiplication rounds"] = Proc.Procp.protocol.rounds;
  statistics["integer dot products"] = Proc.Procp.protocol.dot_counter;
  statistics["probabilistic truncations"] = Proc.Procp.protocol.trunc_pr_counter;
  statistics["probabilistic truncations (big gap)"] = Proc.Procp.protocol.trunc_pr_big_counter;
  statistics["probabilistic truncation rounds"] = Proc.Procp.protocol.trunc_rounds;
  statistics["ANDs"] = Proc.share_thread.protocol->bit_counter;
  statistics["AND rounds"] = Proc.share_thread.protocol->rounds;
  statistics["integer openings"] = MCp->values_opened;
  statistics["integer inputs"] = Proc.Procp.input.values_input;
  for (auto x : Proc.Procp.shuffler.statistics)
    statistics["shuffles of length " + to_string(x.first)] = x.second;

  try
  {
      auto proc = dynamic_cast<RingPrep<Sint>&>(Proc.DataF.data_fp).bit_part_processor;
      if (proc)
        statistics["ANDs in preprocessing"] = proc->protocol.bit_counter;
  }
  catch (...)
  {
  }

  try
  {
      auto protocol = dynamic_cast<BitPrep<Sint>&>(Proc.DataF.data_fp).protocol;
      if (protocol)
        statistics["integer multiplications in preprocessing"] = protocol->counter;
  }
  catch (...)
  {
  }

  // prevent faulty usage message
  Proc.DataF.set_usage(actual_usage);
  auto exe_statistics = Proc.statistics;
  delete processor;

  queues->finished(actual_usage, {P.total_communication(), exe_statistics}, statistics);

  delete MC2;
  delete MCp;
  delete player;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  OPENSSL_thread_stop();
#endif
}


template<class Sint, class Sgf2n>
void* thread_info<Sint, Sgf2n>::main_func(void* ptr)
{
  auto& ti = *(thread_info<Sint, Sgf2n>*)(ptr);
  if (OnlineOptions::singleton.has_option("throw_exceptions"))
    ti.main_func_with_purge();
  else
    {
      try
      {
          ti.main_func_with_purge();
      }
      catch (exception& e)
      {
          cerr << "Fatal error: " << e.what() << endl;
          exit(1);
      }
    }
  return 0;
}

template<class Sint, class Sgf2n>
void thread_info<Sint, Sgf2n>::main_func_with_purge()
{
  auto& ti = *this;
#ifdef INSECURE
  ti.sub_main_func();
#else
  if (ti.machine->opts.live_prep)
    ti.sub_main_func();
  else
    try
    {
      ti.sub_main_func();
    }
    catch (SetupError&)
    {
      throw;
    }
    catch (...)
    {
      purge_preprocessing(machine->get_N(), thread_num);
      throw;
    }
#endif
}


template<class Sint, class Sgf2n>
void thread_info<Sint, Sgf2n>::purge_preprocessing(const Names& N, int thread_num)
{
  cerr << "Purging preprocessed data because something is wrong" << endl;
  try
  {
      DataFiles<Sint, Sgf2n> df(N, thread_num);
      df.purge();
      DataPositions pos;
      SubDataFiles<typename Sint::BitType> bit_df(N, pos, thread_num);
      bit_df.get_part();
      bit_df.purge();
  }
  catch(SetupError&)
  {
  }
  catch(exception& e)
  {
      cerr << "Purging failed. This might be because preprocessed data is incomplete." << endl
          << "SECURITY FAILURE; YOU ARE ON YOUR OWN NOW!" << endl;
      cerr << "Reason: " << e.what() << endl;
  }
}
