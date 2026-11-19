
#pragma once

/* This is a representation of a processing element
 */

#include "numeric/integer.h"
#include "support/exceptions.h"
#include "net/player.h"
#include "vm/data_files.h"
#include "vm/input.h"
#include "vm/private_output.h"
#include "vm/external_clients.h"
#include "vm/binary_file_io.h"
#include "vm/instruction.h"
#include "vm/processor_base.h"
#include "vm/online_options.h"
#include "support/switchable_output.h"
#include "support/check_vector.h"
#include "boolean/processor.h"
#include "boolean/share_thread.h"
#include "protocols/common/secure_shuffle.h"
#include "support/named_stats.h"

class Program;

// synchronize in asymmetric protocols
template<class T>
void sync(vector<Integer>& x, Player& P);

template <class T>
class SubProcessor
{
  StackedVector<typename T::clear> C;
  StackedVector<T> S;

  DataPositions bit_usage;
  NamedStatistics statistics;

  void resize(size_t size)       { C.resize(size); S.resize(size); }

  void matmulsm_prep(int ii, int j, const MemoryPart<T>& source,
      const vector<int>& dim, size_t a, size_t b);
  void matmulsm_finalize(int i, int j, const vector<int>& dim,
      typename vector<T>::iterator C);

  void maybe_check();

  template<class Sint, class Sgf2n> friend class Processor;
  template<class U> friend class SPDZ;
  template<class U> friend class ProtocolBase;
  template<class U> friend class Beaver;

  typedef typename T::BitType::part_type BT;

  typedef typename T::Protocol::Shuffler::StoreType ShuffleStore;
  typedef ShuffleTuple<
      typename T::Protocol::Shuffler::ShuffleType> ShuffleTupleType;

public:
  ArithmeticProcessor* Proc;
  typename T::MacCheck& MC;
  Player& P;
  Preprocessing<T>& DataF;

  typename T::Protocol protocol;
  typename T::Input input;

  typename BT::LivePrep bit_prep;
  vector<typename BT::LivePrep*> personal_bit_preps;

  typename T::Protocol::Shuffler shuffler;

  BinaryFileIo<T> binary_file_io;
  BinaryFileIo<typename T::clear> binary_file_io_clear;

  SubProcessor(ArithmeticProcessor& Proc, typename T::MacCheck& MC,
      Preprocessing<T>& DataF, Player& P);
  SubProcessor(typename T::MacCheck& MC, Preprocessing<T>& DataF, Player& P,
      ArithmeticProcessor* Proc = 0);
  ~SubProcessor();

  void check();

  // Access to PO (via calls to POpen start/stop)
  void POpen(const Instruction& inst);

  void muls(const ArgVector& reg);
  void mulrs(const ArgVector& reg);
  void mulbits(const ArgVector& reg);
  void dotprods(const ArgVector& reg, int size);
  void matmuls(const StackedVector<T>& source, const Instruction& instruction);
  void matmulsm(const MemoryPart<T>& source, const ArgVector& args);

  void matmulsm_finalize_batch(ArgVector::const_iterator start_matmul, int start_i, int start_j,
                               ArgVector::const_iterator end_matmul,
                               int end_i, int end_j);

  void conv2ds(const Instruction& instruction);

  void secure_shuffle(const Instruction& instruction);
  size_t generate_secure_shuffle(const Instruction& instruction,
      ShuffleStore& shuffle_store);
  void apply_shuffle(const Instruction& instruction, ShuffleStore& shuffle_store);
  void inverse_permutation(const Instruction& instruction);

  void input_personal(const ArgVector& args);
  void send_personal(const ArgVector& args);
  void private_output(const ArgVector& args);

  StackedVector<T>& get_s()
  {
    return S;
  }

  StackedVector<typename T::clear>& get_c()
  {
    return C;
  }

  T& get_s_ref(size_t i)
  {
    return S[i];
  }

  typename T::clear& get_c_ref(size_t i)
  {
    return C[i];
  }

  void inverse_permutation(const Instruction &instruction, int handle);

  void push_stack();
  void push_args(const ArgVector& args);
  void pop_stack(const ArgVector& results);

  // Read and write secret numeric data to file (name hardcoded at present)
  template<class U>
  void read_shares_from_file(long start_file_position, int end_file_position_register,
      const ArgVector& data_registers, size_t vector_size, U& Proc);
  void write_shares_to_file(long start_position, const ArgVector& data_registers,
      size_t vector_size);
};

class ArithmeticProcessor : public ProcessorBase
{
protected:
  StackedVector<Integer> Ci;

  ofstream public_output;
  ofstream binary_output;

public:
  int thread_num;

  PRNG secure_prng;
  PRNG shared_prng;

  string private_input_filename;
  string public_input_filename;
  string binary_input_filename;

  ifstream private_input;
  ifstream public_input;
  ifstream binary_input;

  int sent, rounds;

  OnlineOptions opts;

  SwitchableOutput out;

  ArithmeticProcessor() :
      ArithmeticProcessor(OnlineOptions::singleton, BaseMachine::thread_num)
  {
  }
  ArithmeticProcessor(OnlineOptions opts, int thread_num) : thread_num(thread_num),
          sent(0), rounds(0), opts(opts) {}

  virtual ~ArithmeticProcessor()
  {
  }

  bool use_stdin()
  {
    return thread_num == 0 and opts.interactive;
  }

  int get_thread_num()
  {
    return thread_num;
  }

  long read_ci(size_t i) const
    { return Ci[i].get(); }
  Integer& get_ci_ref(size_t i)
    { return Ci[i]; }
  void write_ci(size_t i, const long& x)
    { Ci[i]=x; }
  StackedVector<Integer>& get_ci()
    { return Ci; }

  virtual ofstream& get_public_output()
  {
    throw NotImplemented();
  }
  virtual ofstream& get_binary_output()
  {
    throw NotImplemented();
  }

  void shuffle(const Instruction& instruction);
  void bitdecint(const Instruction& instruction);
};

template<class Sint, class Sgf2n>
class Processor : public ArithmeticProcessor
{
  typedef typename Sint::clear Cint;

  // Data structure used for reading/writing data to/from a socket (i.e. an external party to SPDZ)
  OctetStream socket_stream;

  // avoid re-computation of expensive division
  vector<Cint> inverses2m;

  public:
  DataFiles<Sint, Sgf2n> DataF;
  Player& P;
  typename Sgf2n::MacCheck& MC2;
  typename Sint::MacCheck& MCp;
  Machine<Sint, Sgf2n>& machine;

  GC::ShareThread<typename Sint::BitType> share_thread;
  GC::Processor<typename Sint::BitType> procb;
  SubProcessor<Sgf2n> Proc2;
  SubProcessor<Sint>  Procp;

  unsigned int PC, last_pc;
  TempVars<Sint, Sgf2n> temp;

  ExternalClients& external_clients;

  CommunicationStatistics client_statistics;
  Timer& client_timer;

  void reset(const Program& program,int arg); // Reset the state of the processor
  string get_filename(const char* basename, bool use_number);

  Processor(int thread_num,Player& P,
          typename Sgf2n::MacCheck& MC2,typename Sint::MacCheck& MCp,
          Machine<Sint, Sgf2n>& machine,
          const Program& program);
  ~Processor();

    const typename Sgf2n::clear& read_c2(size_t i) const
      { return Proc2.C[i]; }
    const Sgf2n& read_s2(size_t i) const
      { return Proc2.S[i]; }
    typename Sgf2n::clear& get_c2_ref(size_t i)
      { return Proc2.C[i]; }
    Sgf2n& get_s2_ref(size_t i)
      { return Proc2.S[i]; }
    void write_c2(size_t i,const typename Sgf2n::clear& x)
      { Proc2.C[i]=x; }
    void write_s2(size_t i,const Sgf2n& x)
      { Proc2.S[i]=x; }
  
    const typename Sint::clear& read_cp(size_t i) const
      { return Procp.C[i]; }
    const Sint & read_sp(size_t i) const
      { return Procp.S[i]; }
    typename Sint::clear& get_cp_ref(size_t i)
      { return Procp.C[i]; }
    Sint & get_sp_ref(size_t i)
      { return Procp.S[i]; }
    void write_cp(size_t i,const typename Sint::clear& x)
      { Procp.C[i]=x; }
    void write_sp(size_t i,const Sint & x)
      { Procp.S[i]=x; }

  void check();

  void Dabit(const Instruction& instruction);
  void Edabit(const Instruction& instruction, bool strict = false);

  void convcbitvec(const Instruction& instruction);
  void convcintvec(const Instruction& instruction);
  void convcbit2s(const Instruction& instruction);
  void split(const Instruction& instruction);
  void unsplit(const Instruction& instruction);

  // Access to external client sockets for reading clear/shared data
  void read_socket_ints(int client_id, const ArgVector& registers, int size);

  void write_socket(const RegType reg_type, bool send_macs, int socket_id,
      int message_type, const ArgVector& registers, int size);

  void read_socket_vector(int client_id, const ArgVector& registers,
      int size);
  void read_socket_private(int client_id, const ArgVector& registers,
      int size, bool send_macs);

  Cint get_inverse2(unsigned m);

  void fixinput(const Instruction& instruction);

  // synchronize in asymmetric protocols
  long sync(long x);

  ofstream& get_public_output();
  ofstream& get_binary_output();

  void call_tape(int tape_number, int arg, const ArgVector& results);

  TimerWithCommunication prep_time();

  void jump(int n);

  private:

  template<class T> friend class SPDZ;
  template<class T> friend class SubProcessor;
};
