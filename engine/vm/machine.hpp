#pragma once

#include "vm/machine.h"

#include "vm/memory.hpp"
#include "vm/online_thread.hpp"
#include "vm/online_machine.hpp"
#include "protocols/semi/hemi.hpp"
#include "protocols/fake/fake_stuff.hpp"

#include "support/exceptions.h"

#include <sys/time.h>

#include "numeric/setup.h"
#include "support/mkpath.h"
#include "support/bundle.h"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <pthread.h>
using namespace std;

template<class Sint, class Sgf2n>
void Machine<Sint, Sgf2n>::initialize_binary_domains(int security_parameter, int lg2)
{
  Sgf2n::clear::initialize_field(lg2);

  if (not is_same<typename Sgf2n::MacKeyType, GC::NoValue>())
    {
      if (Sgf2n::MacKeyType::length() < security_parameter)
        {
          cerr << "Security parameter needs to be at most n in GF(2^n)."
              << endl;
          cerr << "Increase the latter (-lg2) or decrease the former (-S)."
              << endl;
          exit(1);
        }
    }

  if (not is_same<typename Sint::BitType::MacKeyType, GC::NoValue>())
    {
      Sint::BitType::MacKeyType::initialize_minimum(security_parameter);
    }
  else
    {
      // Initialize field for CCD
      Sint::BitType::part_type::OpenType::initialize_field();
    }
}

template<class Sint, class Sgf2n>
Machine<Sint, Sgf2n>::Machine(Names& player_names, bool use_encryption,
    const OnlineOptions opts)
  : my_number(player_names.my_num()), N(player_names),
    max_trunc_size(0),
    use_encryption(use_encryption), live_prep(opts.live_prep), opts(opts),
    external_clients(my_number)
{
  OnlineOptions::singleton = opts;

  int min_players = 3 - Sint::dishonest_majority;
  if (Sint::is_real)
    {
      if (N.num_players() == 1)
        {
          cerr << "Need more than one player to run a protocol." << endl;
          cerr << "Use 'emulate.x' for just running the virtual machine" << endl;
          exit(1);
        }
      else if (N.num_players() < min_players)
        {
          cerr << "Need at least " << min_players << " players for this protocol."
              << endl;
          exit(1);
        }
    }

  // Set the prime modulus from command line or program if applicable
  if (opts.prime)
    Sint::clear::initialize_field(opts.prime);

  initialize_binary_domains(opts.security_parameter, opts.lg2);

  // make directory for outputs if necessary
  mkdir_p(PREP_DIR);

  string id = "machine";
  if (use_encryption)
    P = new CryptoPlayer(N, id);
  else
    P = new PlainPlayer(N, id);

  if (opts.live_prep)
    {
      Sint::LivePrep::basic_setup(*P);
    }

  // Set the prime modulus if not done earlier
  if (not Sint::clear::length())
    Sint::clear::read_or_generate_setup(prep_directory_prefix<Sint>(), opts);

  Sint::MacCheck::setup(*P);
  Sint::BitType::MacCheck::setup(*P);
  Sgf2n::MacCheck::setup(*P);

  // for OT-based preprocessing
  Sint::clear::next::template init<typename Sint::clear>(false);

  if (opts.live_prep)
    {
      alphapi = Sint::LivePrep::get_mac_key(*P);
      alpha2i = Sgf2n::LivePrep::get_mac_key(*P, true);
      alphabi = Sint::BitType::LivePrep::get_mac_key(*P);
    }
  else
    {
      // check for directory
      SubDataFiles<Sint>::check_setup(N);
      // require existing MAC keys
      if (Sint::has_mac)
        maybe_read_mac_key<Sint>(N, alphapi);
      if (Sgf2n::has_mac)
        maybe_read_mac_key<Sgf2n>(N, alpha2i);
      if (Sint::BitType::part_type::has_mac)
        maybe_read_mac_key<typename Sint::BitType::part_type>(N, alphabi);
    }

#ifdef DEBUG_MAC
  cerr << "MAC Key p = " << alphapi << endl;
  cerr << "MAC Key 2 = " << alpha2i << endl;
#endif

  // Initialize the global memory
  auto memtype = opts.memtype;
  if (memtype.compare("old")==0)
     {
       if (Sint::real_shares(*P))
         {
           ifstream inpf;
           inpf.open(memory_filename(), ios::in | ios::binary);
           if (inpf.fail()) { throw file_error(memory_filename()); }
           inpf >> M2 >> Mp >> mi;
           if (inpf.get() != 'M')
           {
               cerr << "Invalid memory file. Run with '-m empty'." << endl;
               exit(1);
           }
           inpf.close();
         }
     }
  else if (!(memtype.compare("empty")==0))
     { cerr << "Invalid memory argument" << endl;
       exit(1);
     }
}

template<class Sint, class Sgf2n>
void Machine<Sint, Sgf2n>::prepare(const string& progname_str)
{
  int old_n_threads = nthreads;
  progs.clear();
  load_schedule(progname_str);

  if (opts.verbose)
    suggest_optimizations();

  check_program();

  // keep preprocessing
  nthreads = max(old_n_threads, nthreads);

  // initialize persistence if necessary
  for (auto& prog : progs)
    {
      if (prog.writes_persistence)
        {
          BinaryFileIo<Sint>::reset(my_number);
          BinaryFileIo<Sgf2n>::reset(my_number);
          BinaryFileIo<typename Sint::clear>::reset(my_number);
          break;
        }
    }

#ifdef VERBOSE
  progs[0].print_offline_cost();
#endif

  /* Set up the threads */
  tinfo.resize(nthreads);
  queues.resize(nthreads);
  join_timer.resize(nthreads);
  assert(threads.size() == size_t(old_n_threads));

  for (int i = old_n_threads; i < nthreads; i++)
    {
      queues[i] = new ThreadQueue;
      // stand-in for initialization
      queues[i]->schedule({});
      tinfo[i].thread_num=i;
      tinfo[i].nms=&N;
      tinfo[i].alphapi=&alphapi;
      tinfo[i].alpha2i=&alpha2i;
      tinfo[i].machine=this;
      pthread_t thread;
      int res = pthread_create(&thread, NULL,
          thread_info<Sint, Sgf2n>::main_func, &tinfo[i]);

      if (res == 0)
        threads.push_back(thread);
      else
        throw runtime_error("cannot start thread");

      // cannot start threads in parallel
      queues[i]->result();
    }

  assert(queues.size() == threads.size());
}

template<class Sint, class Sgf2n>
Machine<Sint, Sgf2n>::~Machine()
{
  stop_threads();

  Sint::LivePrep::teardown();
  Sgf2n::LivePrep::teardown();

  Sint::MacCheck::teardown();
  Sint::BitType::MacCheck::teardown();
  Sgf2n::MacCheck::teardown();

  delete P;
}

template<class Sint, class Sgf2n>
size_t Machine<Sint, Sgf2n>::load_program(const string& threadname,
    const string& filename)
{
  progs.push_back(N.num_players());
  int i = progs.size() - 1;
  progs[i].parse(filename);
  M2.minimum_size(SGF2N, CGF2N, progs[i], threadname);
  Mp.minimum_size(SINT, CINT, progs[i], threadname);
  mi.minimum_size(NONE, INT, progs[i], threadname);
  bit_memories.reset(progs[i]);
  return progs.back().size();
}

template<class Sint, class Sgf2n>
DataPositions Machine<Sint, Sgf2n>::run_tapes(const ArgVector& args,
    DataFiles<Sint, Sgf2n>& DataF)
{
  assert(args.size() % 3 == 0);
  size_t max_thread = 0;
  for (unsigned i = 0; i < args.size(); i += 3)
  {
    fill_buffers(args[i], args[i + 1], &DataF.data_fp, &DataF.data_fb);
    if (max_thread < args[i])
      max_thread = args[i];
  }
  queues.set_n_tape_threads(max_thread + 1);

  DataPositions res(N.num_players());
  for (unsigned i = 0; i < args.size(); i += 3)
    res.increase(
        run_tape(args[i], args[i + 1], args[i + 2], DataF.tellg() + res));
  DataF.skip(res);
  return res;
}

template<class Sint, class Sgf2n>
void Machine<Sint, Sgf2n>::fill_buffers(int thread_number, int tape_number,
    Preprocessing<Sint>* prep,
    Preprocessing<typename Sint::BitType>* bit_prep)
{
  // central preprocessing
  auto usage = progs[tape_number].get_offline_data_used();
  if (Sint::expensive and prep != 0 and OnlineOptions::singleton.bucket_size == 3)
    {
      try
      {
          auto& source = *dynamic_cast<BufferPrep<Sint>*>(prep);
          auto& dest =
              dynamic_cast<BufferPrep<Sint>&>(tinfo[thread_number].processor->DataF.data_fp);
          for (auto it = usage.edabits.begin(); it != usage.edabits.end(); it++)
            {
              bool strict = it->first.first;
              int n_bits = it->first.second;
              size_t required = DIV_CEIL(it->second,
                  Sint::BitType::part_type::default_length);
              auto& destination_buffer = dest.edabits[it->first];
              auto& source_buffer = source.edabits[it->first];
              while (destination_buffer.size() < required)
                {
                  if (source_buffer.empty())
                    source.buffer_edabits(strict, n_bits, &queues);
                  size_t n = min(source_buffer.size(),
                      required - destination_buffer.size());
                  destination_buffer.insert(destination_buffer.end(), source_buffer.end() - n,
                      source_buffer.end());
                  source_buffer.erase(source_buffer.end() - n,
                      source_buffer.end());
                }
            }
      }
      catch (bad_cast& e)
      {
#ifdef VERBOSE_CENTRAL
        cerr << "Problem with central preprocessing" << endl;
#endif
      }
    }

  typedef typename Sint::BitType BitType;
  if (BitType::expensive_triples and bit_prep and OnlineOptions::singleton.bucket_size == 3)
    {
      try
      {
          auto& source = *dynamic_cast<BufferPrep<BitType>*>(bit_prep);
          auto &dest =
              dynamic_cast<BufferPrep<BitType>&>(tinfo[thread_number].processor->share_thread.DataF);
          for (int i = 0; i < DIV_CEIL(usage.files[DATA_GF2][DATA_TRIPLE],
                                        BitType::default_length); i++)
            dest.push_triple(source.get_triple_no_count(BitType::default_length));
      }
      catch (bad_cast& e)
      {
#ifdef VERBOSE_CENTRAL
        cerr << "Problem with central bit triple preprocessing: " << e.what() << endl;
#endif
      }
    }

  if (not HemiOptions::singleton.plain_matmul)
    fill_matmul(thread_number, tape_number, prep, Sint::triple_matmul);
}

template<class Sint, class Sgf2n>
template<int>
void Machine<Sint, Sgf2n>::fill_matmul(int thread_number, int tape_number,
    Preprocessing<Sint>* prep, true_type)
{
  auto usage = progs[tape_number].get_offline_data_used();
  for (auto it = usage.matmuls.begin(); it != usage.matmuls.end(); it++)
    {
      try
      {
          auto& source_processor = *dynamic_cast<BufferPrep<Sint>&>(*prep).proc;
          int max_inner = opts.batch_size;
          int max_columns = opts.batch_size;
          for (int j = 0; j < it->first[1]; j += max_inner)
            {
              for (int k = 0; k < it->first[2]; k += max_columns)
                {
                  auto subdim = it->first;
                  subdim[1] = min(subdim[1] - j, max_inner);
                  subdim[2] = min(subdim[2] - k, max_columns);
                  auto& source_protocol = dynamic_cast<Hemi<Sint>&>(source_processor.protocol);
                  auto& source = source_protocol.get_matrix_prep(
                          subdim, source_processor);
                  auto& dest =
                      dynamic_cast<Hemi<Sint>&>(tinfo[thread_number].processor->Procp.protocol).get_matrix_prep(
                          subdim, tinfo[thread_number].processor->Procp);
                  if (not source_protocol.use_plain_matmul(subdim, source_processor))
                    for (int i = 0; i < it->second; i++)
                      dynamic_cast<BufferPrep<ShareMatrix<Sint>>&>(dest).push_triple(
                          source.get_triple_no_count(-1));
                }
            }
      }
      catch (bad_cast& e)
      {
#ifdef VERBOSE_CENTRAL
        cerr << "Problem with central matmul preprocessing: " << e.what() << endl;
#endif
      }
    }
}

template<class Sint, class Sgf2n>
DataPositions Machine<Sint, Sgf2n>::run_tape(int thread_number, int tape_number,
    int arg, const DataPositions& pos)
{
  if (size_t(thread_number) >= tinfo.size())
    throw overflow("invalid thread number", thread_number, tinfo.size());
  if (size_t(tape_number) >= progs.size())
    throw overflow("invalid tape number", tape_number, progs.size());

  if (BaseMachine::thread_num == 0)
    active_threads.insert(thread_number);

  queues[thread_number]->schedule({tape_number, arg, pos});
  //printf("Send signal to run program %d in thread %d\n",tape_number,thread_number);
  //printf("Running line %d\n",exec);
  if (progs[tape_number].usage_unknown())
    {
      if (not opts.live_prep and thread_number != 0)
        {
          insecure(
              "Internally called tape " + to_string(tape_number)
                  + " has unknown offline data usage");
        }
      return DataPositions(N.num_players());
    }
  else
    {
      // Bits, Triples, Squares, and Inverses skipping
      return progs[tape_number].get_offline_data_used();
    }
}

template<class Sint, class Sgf2n>
DataPositions Machine<Sint, Sgf2n>::join_tape(int i)
{
  join_timer[i].start();
  //printf("Waiting for client to terminate\n");
  auto pos = queues[i]->result().pos;
  join_timer[i].stop();
  if (BaseMachine::thread_num == 0)
  {
      active_threads.erase(i);
      if (active_threads == set({0}))
          queues.set_n_tape_threads(1);
  }
  return pos;
}

template<class Sint, class Sgf2n>
void Machine<Sint, Sgf2n>::run_step(const string& progname)
{
  prepare(progname);
  run_tape(0, 0, 0, N.num_players());
  join_tape(0);
}

template<class Sint, class Sgf2n>
void Machine<Sint, Sgf2n>::run_function(const string& name,
        FunctionArgument& result, vector<FunctionArgument>& arguments)
{
  ifstream file;
  FunctionArgument::open(file, name, arguments);

  string progname, return_type;
  int tape_number, return_reg;
  file >> progname >> tape_number >> return_type >> return_reg;

  result.CheckType(return_type);

  vector<int> arg_registers(arguments.size());
  vector<int> address_registers(arguments.size());
  for (size_t i = 0; i < arguments.size(); i++)
    {
      file >> arg_registers.at(i);
      if (arguments[i].get_memory())
        file >> address_registers.at(i);
    }

  if (not file.good())
    throw runtime_error("error reading file for function " + name);

  prepare(progname);
  auto& processor = *tinfo.at(0).processor;
  processor.reset(progs.at(tape_number), 0);

  for (size_t i = 0; i < arguments.size(); i++)
    for (size_t j = 0; j < arguments[i].get_size(); j++)
      {
        if (arguments[i].get_n_bits())
          {
            size_t n_limbs = DIV_CEIL(arguments[i].get_n_bits(),
                Sint::BitType::default_length);
            for (size_t k = 0; k < n_limbs; k++)
              bit_memories.MS[arg_registers.at(i) + j * n_limbs + k] =
                  arguments[i].get_value<vector<typename Sint::BitType>>(j).at(
                      k);
          }
        else if (arguments[i].has_reg_type("s"))
          {
            auto& value = arguments[i].get_value<Sint>(j);
            if (arguments[i].get_memory())
              Mp.MS[arg_registers.at(i) + j] = value;
            else
              processor.Procp.get_s()[arg_registers.at(i) + j] = value;
          }
        else
          {
            assert(arguments[i].has_reg_type("ci"));
            processor.write_ci(arg_registers.at(i) + j, arguments[i].get_value<long>(j));
          }
        if (arguments[i].get_memory())
          processor.write_ci(address_registers.at(i), arg_registers.at(i));
      }

  run_tape(0, tape_number, 0, N.num_players());
  join_tape(0);

  assert(result.has_reg_type("s"));
  for (size_t j = 0; j < result.get_size(); j++)
    result.get_value<Sint>(j) = processor.Procp.get_s()[return_reg + j];

  for (size_t i = 0; i < arguments.size(); i++)
    if (arguments[i].get_memory())
      for (size_t j = 0; j < arguments[i].get_size(); j++)
        {
          if (arguments[i].get_n_bits())
            {
              size_t n_limbs = DIV_CEIL(arguments[i].get_n_bits(),
                  Sint::BitType::default_length);
              for (size_t k = 0; k < n_limbs; k++)
                arguments[i].get_value<vector<typename Sint::BitType>>(j).at(k) =
                    bit_memories.MS[arg_registers.at(i) + j * n_limbs + k];
            }
          else
            arguments[i].get_value<Sint>(j) = Mp.MS[arg_registers.at(i) + j];
        }
}

template<class Sint, class Sgf2n>
pair<DataPositions, NamedCommunicationStatistics> Machine<Sint, Sgf2n>::stop_threads()
{
  // only stop actually running threads
  nthreads = threads.size();

  // Tell all C-threads to stop
  for (int i=0; i<nthreads; i++)
    {
      //printf("Send kill signal to client\n");
      auto queue = queues.at(i);
      assert(queue);
      queue->schedule(-1);
    }

  // sum actual usage
  DataPositions pos(N.num_players());

#ifdef DEBUG_THREADS
  cerr << "Waiting for all clients to finish" << endl;
#endif
  // Wait until all clients have signed out
  for (int i=0; i<nthreads; i++)
    {
      queues[i]->schedule({});
      pos.increase(queues[i]->result().pos);
      pthread_join(threads[i],NULL);
    }

  auto statistics = total_statistics();
  communication_statistics = statistics.communication_statistics;
  this->statistics = statistics.exe_statistics;
  max_communication = queues.max_communication();

  if (OnlineOptions::singleton.verbose)
    {
      NamedStatistics total;
      for (auto queue : queues)
        total += queue->statistics;
      total.print();
      queues.print_breakdown();
    }

  for (auto& queue : queues)
    if (queue)
      delete queue;

  queues.clear();
  threads.clear();

  nthreads = 0;

  return {pos, communication_statistics};
}

template<class Sint, class Sgf2n>
void Machine<Sint, Sgf2n>::run(const string& progname)
{
  prepare(progname);

  if (opts.verbose and setup_timer.is_running())
    {
      cerr << "Setup took " << setup_timer.elapsed() << " seconds." << endl;
      setup_timer.stop();
    }

  Timer processor_timer(CLOCK_PROCESS_CPUTIME_ID);
  processor_timer.start();
  timer[0].start({});

  // run main tape
  run_tape(0, 0, 0, N.num_players());
  join_tape(0);

  print_compiler();

  finish_timer.start();

  // actual usage
  multithread = nthreads > 1;
  auto res = stop_threads();
  DataPositions& pos = res.first;

  finish_timer.stop();
  
#ifdef VERBOSE
  cerr << "Memory usage: ";
  tinfo[0].print_usage(cerr, Mp.MS, "sint");
  tinfo[0].print_usage(cerr, Mp.MC, "cint");
  tinfo[0].print_usage(cerr, M2.MS, "sgf2n");
  tinfo[0].print_usage(cerr, M2.MS, "cgf2n");
  tinfo[0].print_usage(cerr, bit_memories.MS, "sbits");
  tinfo[0].print_usage(cerr, bit_memories.MC, "cbits");
  tinfo[0].print_usage(cerr, mi.MC, "regint");
  cerr << endl;

  for (unsigned int i = 0; i < join_timer.size(); i++)
    cerr << "Join timer: " << i << " " << join_timer[i].elapsed() << endl;
  cerr << "Finish timer: " << finish_timer.elapsed() << endl;
#endif

  NamedCommunicationStatistics& communication_statistics = res.second;

  if (opts.verbose)
    {
      cerr << "Communication details";
      if (multithread)
        cerr << " (rounds and time in parallel threads counted double)";
      cerr << ":" << endl;
      communication_statistics.print(false, max_communication);
      cerr << "CPU time = " <<  processor_timer.elapsed();
      if (multithread)
        cerr << " (overall core time)";
      cerr << endl;
      auto& P = *this->P;
      auto one_off = TreeSum<Z2<64>>().run(
          this->one_off_communication.sent, P).get_limb(0);
      if (one_off)
        cerr << "One-off global communication: " << one_off * 1e-6 << " MB"
            << endl;
    }

  print_timers();

  if (Sint::is_real)
    this->print_communication(*this->P, communication_statistics);

#ifdef VERBOSE_OPTIONS
  if (opening_sum < N.num_players() && !direct)
    cerr << "Summed at most " << opening_sum << " shares at once with indirect communication" << endl;
  else
    cerr << "Summed all shares at once" << endl;

  if (max_broadcast < N.num_players() && !direct)
    cerr << "Send to at most " << max_broadcast << " parties at once" << endl;
  else
    cerr << "Full broadcast" << endl;
#endif

  if (not OnlineOptions::singleton.has_option("output_full_memory")
      and OnlineOptions::singleton.disk_memory.empty())
    {
      // Reduce memory size to speed up
      unsigned max_size = 1 << 20;
      if (M2.size_s() > max_size)
        M2.resize_s(max_size);
      if (Mp.size_s() > max_size)
        Mp.resize_s(max_size);
    }

  if (Sint::real_shares(*P) and not opts.has_option("no_memory_output")
      and (OnlineOptions::singleton.disk_memory.empty()
          or opts.has_option("memory_output")))
    {
      RunningTimer timer;
      // Write out the memory to use next time
      ofstream outf(memory_filename(), ios::out | ios::binary);
      outf << M2 << Mp << mi;
      outf << 'M';
      outf.close();

      bit_memories.write_memory(N.my_num());

      if (opts.has_option("time_memory_output"))
        cerr << "Writing memory to disk took " << timer.elapsed() << " seconds"
            << endl;
    }

  if (opts.verbose)
    {
      cerr << "Actual preprocessing cost of program:" << endl;
      pos.print_cost();
    }

  if (pos.any_more(progs[0].get_offline_data_used())
      and not progs[0].usage_unknown())
    throw runtime_error("computation used more preprocessing than expected");

  if (not statistics.empty() and opts.verbose)
    {
      statistics.print();
    }

  if (not opts.file_prep_per_thread)
    {
      DataFiles<Sint, Sgf2n> df(*this);
      df.seekg(pos);
      df.prune();
    }

  suggest_optimizations();

  if (N.num_players() > 4)
    {
      string alt = Sint::alt();
      if (alt.size())
        cerr << "This protocol doesn't scale well with the number of parties, "
            << "have you considered using " << alt << " instead?" << endl;
    }

  if ((nan_warning or mini_warning) and Sint::real_shares(*P))
    {
      if (nan_warning)
        cerr << "Outputs of 'NaN' might be related to exceeding the sfix range. ";
      if (mini_warning)
        cerr << pow(2, mini_warning) << " is the smallest non-zero number "
            << "in a used fixed-point representation. ";
      cerr << "See https://mp-spdz.readthedocs.io/en/latest/Compiler.html#Compiler.types.sfix";
      cerr << " for details" << endl;
      nan_warning = false;
      mini_warning = 0;
    }

#ifdef INSECURE
  if (not opts.live_prep)
    cerr << "WARNING: Preprocessing data in files left untouched. "
        << "Next run will use the same randomness." << endl;
#endif

#ifdef VERBOSE
  cerr << "End of prog" << endl;
#endif
}

template<class Sint, class Sgf2n>
string Machine<Sint, Sgf2n>::memory_filename()
{
  return BaseMachine::memory_filename(Sint::type_short(), my_number);
}

template<class Sint, class Sgf2n>
template<class T>
string Machine<Sint, Sgf2n>::prep_directory_prefix()
{
  return opts.prep_directory_prefix<T>(N.num_players());
}

template<class Sint, class Sgf2n>
void Machine<Sint, Sgf2n>::reqbl(int n)
{
  Sint::clear::reqbl(n);
}

template<class Sint, class Sgf2n>
void Machine<Sint, Sgf2n>::active(int n)
{

  if (Sint::malicious and n == 0)
    {
      cerr << "Program requires a semi-honest protocol" << endl;
      exit(1);
    }
}

template<class Sint, class Sgf2n>
void Machine<Sint, Sgf2n>::suggest_optimizations()
{
  string optimizations;
  if (relevant_options.find("trunc_pr") != string::npos and Sint::has_trunc_pr)
    optimizations.append("\tprogram.use_trunc_pr = True\n");
  if (relevant_options.find("split") != string::npos and Sint::has_split)
    optimizations.append(
        "\tprogram.use_split(" + to_string(N.num_players()) + ")\n");
  if (relevant_options.find("edabit") != string::npos and not Sint::has_split and Sint::is_real)
    optimizations.append("\tprogram.use_edabit(True)\n");
  if (not optimizations.empty())
    cerr << "This program might benefit from some protocol options." << endl
        << "Consider adding the following at the beginning of your code:"
        << endl << optimizations;
  if (Sint::clear::n_bits() < max_trunc_size)
    cerr << "The computation domain is too small "
        << "for low-round truncation; it would need to have at least "
        << max_trunc_size << " bits." << endl;
#ifndef __clang__
  cerr << "This virtual machine was compiled with GCC. Recompile with "
      "'CXX = clang++' in 'CONFIG.mine' for optimal performance." << endl;
#endif
}

template<class Sint, class Sgf2n>
void Machine<Sint, Sgf2n>::check_program()
{
  Hash hasher;
  for (auto& prog : progs)
    hasher.update(prog.get_hash());
  assert(P);
  Bundle<OctetStream> bundle(*P);
  hasher.final(bundle.mine);
  try
  {
    bundle.compare(*P);
  }
  catch (MismatchAmongParties&)
  {
    throw runtime_error("program differs between parties");
  }
}

template<class Sint, class Sgf2n>
void Machine<Sint, Sgf2n>::gap_warning(int k)
{
  if (k > max_trunc_size)
    {
      warn_lock.lock();
      max_trunc_size = max(k, max_trunc_size);
      warn_lock.unlock();
    }
}
