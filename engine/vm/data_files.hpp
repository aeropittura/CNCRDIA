#pragma once

#include "vm/data_files.h"
#include "vm/processor.h"
#include "vm/no_file_prep.h"
#include "protocols/common/dabit.h"
#include "numeric/setup.h"
#include "boolean/bit_prep_files.h"
#include "support/benchmarking.h"

template<class T>
Preprocessing<T>* Preprocessing<T>::get_live_prep(SubProcessor<T>* proc,
    DataPositions& usage)
{
  return new typename T::LivePrep(proc, usage);
}

template<class T>
template<class U, class V>
Preprocessing<T>* Preprocessing<T>::get_new(
    Machine<U, V>& machine,
    DataPositions& usage, SubProcessor<T>* proc)
{
  if (machine.live_prep)
    return get_live_prep(proc, usage);
  else
    return new SubDataFiles<T>(machine.get_N(),
        machine.template prep_directory_prefix<T>(), usage, BaseMachine::thread_num);
}

template<class T>
template<int>
Preprocessing<T>* Preprocessing<T>::get_new(
    bool live_prep, const Names& N,
    DataPositions& usage)
{
  if (live_prep)
    return new typename T::LivePrep(usage);
  else
    return new GC::BitPrepFiles<T>(N,
        get_prep_sub_directory<T>(PREP_DIR, N.num_players()), usage,
        BaseMachine::thread_num);
}

template<class T>
T Preprocessing<T>::get_random_from_inputs(int nplayers)
{
  T res;
  for (int j = 0; j < nplayers; j++)
    {
      T tmp;
      typename T::OpenType _;
      this->get_input_no_count(tmp, _, j);
      res += tmp;
    }
  return res;
}

template<class T>
SubDataFiles<T>::SubDataFiles(const Names& N, DataPositions& usage,
    int thread_num) :
    SubDataFiles(N, get_prep_directory(N), usage,
        thread_num)
{
}


template<class T>
int SubDataFiles<T>::tuple_length(int dtype)
{
  return DataPositions::tuple_size[dtype] * T::size();
}

template<class T>
string SubDataFiles<T>::get_filename(const Names& N, Dtype type,
    int thread_num)
{
  return PrepBase::get_filename(get_prep_sub_directory<T>(N.num_players()),
      type, T::type_short(), N.my_num(), thread_num);
}

template<class T>
string SubDataFiles<T>::get_input_filename(const Names& N, int input_player,
    int thread_num)
{
  return PrepBase::get_input_filename(
      get_prep_sub_directory<T>(N.num_players()), T::type_short(), input_player,
      N.my_num(), thread_num);
}

template<class T>
string SubDataFiles<T>::get_edabit_filename(const Names& N, int n_bits,
    int thread_num)
{
  return PrepBase::get_edabit_filename(
      get_prep_sub_directory<T>(N.num_players()), n_bits, N.my_num(), thread_num);
}

template<class T>
string SubDataFiles<T>::get_prep_directory(const Names& N)
{
  return OnlineOptions::singleton.prep_directory_prefix<T>(N.num_players());
}

template<class T>
void SubDataFiles<T>::check_setup(const Names& N)
{
  return check_setup(N.num_players(), get_prep_directory(N));
}

template<class T>
void SubDataFiles<T>::check_setup(int num_players, const string& prep_directory)
{
  if (T::function_dependent)
    throw runtime_error("preprocessing from file not implemented "
        "for function-dependent preprocessing");

  try
    {
      T::clear::check_setup(prep_directory);
    }
  catch (exception& e)
    {
      throw PrepSetupError(e.what(), num_players,
          T::template protocol_fake_options<typename T::clear>());
    }
}

template<class T>
SubDataFiles<T>::SubDataFiles(int my_num, int num_players,
    const string& prep_data_directory, DataPositions& usage, int thread_num) :
    Preprocessing<T>(usage),
    my_num(my_num), num_players(num_players), prep_data_directory(prep_data_directory),
    thread_num(thread_num), part(0)
{
#ifdef DEBUG_FILES
  cerr << "Setting up Data_Files in: " << prep_data_directory << endl;
#endif

  check_setup(num_players, prep_data_directory);

  string type_short = T::type_short();
  string type_string = T::type_string();

  for (int dtype = 0; dtype < N_DTYPE; dtype++)
    {
      if (T::clear::allows(Dtype(dtype)))
        {
          buffers[dtype].setup(num_players,
              PrepBase::get_filename(prep_data_directory, Dtype(dtype), type_short,
                  my_num, thread_num), tuple_length(dtype), type_string,
              DataPositions::dtype_names[dtype]);
        }
    }

  dabit_buffer.setup(num_players,
      PrepBase::get_filename(prep_data_directory, DATA_DABIT,
          type_short, my_num, thread_num), Dabit<T>::size(), type_string,
      DataPositions::dtype_names[DATA_DABIT]);

  input_buffers.resize(num_players);
  for (int i=0; i<num_players; i++)
    {
      string filename = PrepBase::get_input_filename(prep_data_directory,
          type_short, i, my_num, thread_num);
      if (i == my_num)
        my_input_buffers.setup(num_players, filename,
            InputTuple<T>::size(), type_string);
      else
        input_buffers[i].setup(num_players, filename,
            T::size(), type_string);
    }

#ifdef DEBUG_FILES
  cerr << "done\n";
#endif
}

template<class Sint, class Sgf2n>
DataFiles<Sint, Sgf2n>::DataFiles(Machine<Sint, Sgf2n>& machine, SubProcessor<Sint>* procp,
    SubProcessor<Sgf2n>* proc2) :
    usage(machine.get_N().num_players()),
    data_fp(*Preprocessing<Sint>::get_new(machine, usage, procp)),
    data_f2(*Preprocessing<Sgf2n>::get_new(machine, usage, proc2)),
    data_fb(
        *Preprocessing<typename Sint::BitType>::get_new(machine.live_prep,
            machine.get_N(), usage))
{
}

template<class Sint, class Sgf2n>
DataFiles<Sint, Sgf2n>::DataFiles(const Names& N, int thread_num) :
    usage(N.num_players()),
    data_fp(*new SubDataFiles<Sint>(N, usage, thread_num)),
    data_f2(*new SubDataFiles<Sgf2n>(N, usage, thread_num)),
    data_fb(*new SubDataFiles<typename Sint::BitType>(N, usage, thread_num))
{
}


template<class Sint, class Sgf2n>
DataFiles<Sint, Sgf2n>::~DataFiles()
{
  delete &data_fp;
  delete &data_f2;
  delete &data_fb;
}

template<class T>
SubDataFiles<T>::~SubDataFiles()
{
  if (part != 0)
    delete part;
}

template<class T>
long SubDataFiles<T>::additional_inputs(const DataPositions& usage)
{
  auto& domain_usage = usage.files[T::clear::field_type()];
  long add_to_inputs = domain_usage[DATA_RANDOM];
  if (T::randoms_for_opens)
    add_to_inputs += domain_usage[DATA_OPEN];
  return add_to_inputs;
}

template<class T>
void SubDataFiles<T>::seekg(DataPositions& pos)
{
  if (OnlineOptions::singleton.file_prep_per_thread)
    return;

  if (T::LivePrep::use_part)
    {
      get_part().seekg(pos);
      return;
    }

  DataFieldType field_type = T::clear::field_type();
  for (int dtype = 0; dtype < N_DTYPE; dtype++)
    if (T::clear::allows(Dtype(dtype)))
      buffers[dtype].seekg(pos.files[field_type][dtype]);

  long add_to_inputs = additional_inputs(pos);

  for (int j = 0; j < num_players; j++)
    if (j == my_num)
      my_input_buffers.seekg(pos.inputs[j][field_type] + add_to_inputs);
    else
      input_buffers[j].seekg(pos.inputs[j][field_type] + add_to_inputs);

  for (map<DataTag, long long>::const_iterator it = pos.extended[field_type].begin();
      it != pos.extended[field_type].end(); it++)
    {
      setup_extended(it->first);
      extended[it->first].seekg(it->second);
    }
  dabit_buffer.seekg(pos.files[field_type][DATA_DABIT]);

  if (field_type == DATA_INT)
    {
      for (auto& x : pos.edabits)
        {
          // open files
          get_edabit_buffer(x.first.second);
        }


      int block_size = Edabitvec<T>::MAX_SIZE;
      for (auto& x : edabit_buffers)
        {
          int n = pos.edabits[{true, x.first}] + pos.edabits[{false, x.first}];
          x.second.seekg(n / block_size);
          Edabit<T> eb;
          for (int i = 0; i < n % block_size; i++)
            get_edabit_no_count(false, x.first, eb);
        }
    }
}

template<class Sint, class Sgf2n>
void DataFiles<Sint, Sgf2n>::seekg(DataPositions& pos)
{
  data_fp.seekg(pos);
  data_f2.seekg(pos);
  data_fb.seekg(pos);
  usage = pos;
}

template<class Sint, class Sgf2n>
void DataFiles<Sint, Sgf2n>::skip(const DataPositions& pos)
{
  DataPositions new_position = usage;
  new_position.increase(pos);
  skipped.increase(pos);
  seekg(new_position);
}

template<class T>
void SubDataFiles<T>::prune()
{
  for (auto& buffer : buffers)
    buffer.prune();
  my_input_buffers.prune();
  for (int j = 0; j < num_players; j++)
    input_buffers[j].prune();
  for (auto& it : extended)
    it.second.prune();
  dabit_buffer.prune();
  if (part != 0)
    part->prune();
  for (auto& x : edabit_buffers)
    x.second.prune();
}

template<class Sint, class Sgf2n>
void DataFiles<Sint, Sgf2n>::prune()
{
  data_fp.prune();
  data_f2.prune();
  data_fb.prune();
}

template<class T>
void SubDataFiles<T>::purge()
{
  for (auto& buffer : buffers)
    buffer.purge();
  my_input_buffers.purge();
  for (int j = 0; j < num_players; j++)
    input_buffers[j].purge();
  for (auto it : extended)
    it.second.purge();
  dabit_buffer.purge();
  if (part != 0)
    part->purge();
  for (auto& x : edabit_buffers)
    x.second.prune();
}

template<class T>
void SubDataFiles<T>::setup_extended(const DataTag& tag, int tuple_size)
{
  auto& buffer = extended[tag];
  int tuple_length = tuple_size * T::size();

  if (!buffer.is_up())
    {
      stringstream ss;
      ss << prep_data_directory << tag.get_string() << "-" << T::type_short() << "-P" << my_num;
      buffer.setup(num_players, ss.str(), tuple_length);
    }

  buffer.check_tuple_length(tuple_length);
}

template<class T>
void SubDataFiles<T>::get_no_count(StackedVector<T>& S, DataTag tag, const ArgVector& regs, int vector_size)
{
  setup_extended(tag, regs.size());
  for (int j = 0; j < vector_size; j++)
    for (unsigned int i = 0; i < regs.size(); i++)
      extended[tag].input(S[regs[i] + j]);
}

template<class T>
void SubDataFiles<T>::get_dabit_no_count(T& a, typename T::BitType& b)
{
  Dabit<T> tmp;
  dabit_buffer.input(tmp);
  a = tmp.first;
  b = tmp.second;
}

template<class T>
EdabitBuffer<T>& SubDataFiles<T>::get_edabit_buffer(int n_bits)
{
  if (edabit_buffers.find(n_bits) == edabit_buffers.end())
    {
      string filename = PrepBase::get_edabit_filename(prep_data_directory,
          n_bits, my_num, thread_num);
      edabit_buffers[n_bits] = n_bits;
      edabit_buffers[n_bits].setup(filename,
          T::size() * Edabitvec<T>::MAX_SIZE
              + n_bits * T::BitType::part_type::size());
    }
  return edabit_buffers[n_bits];
}

template<class T>
Edabitvec<T> SubDataFiles<T>::get_edabitvec(bool strict, int n_bits)
{
  if (my_edabits[n_bits].empty())
    return get_edabit_buffer(n_bits).read();
  else
    {
      auto res = my_edabits[n_bits];
      my_edabits[n_bits] = {};
      this->fill(res, strict, n_bits);
      return res;
    }
}

template<class T>
void Preprocessing<T>::fill(Edabitvec<T>& res, bool strict, int n_bits)
{
  Edabit<T> eb;
  while (res.size() < res.MAX_SIZE)
    {
      get_edabit_no_count(strict, n_bits, eb);
      res.push_back(eb);
    }
}

template<class T>
typename SubDataFiles<T>::part_type& SubDataFiles<T>::get_part()
{
  if (part == 0)
    part = new part_type(my_num, num_players,
        get_prep_sub_directory<typename T::part_type>(num_players), this->usage,
        thread_num);
  return *part;
}

template<class Sint, class Sgf2n>
TimerWithCommunication DataFiles<Sint, Sgf2n>::total_time()
{
  return data_fp.total_time() + data_f2.total_time() + data_fb.total_time();
}

template<class T>
TimerWithCommunication Preprocessing<T>::total_time()
{
  auto res = prep_timer;
  try
  {
    res += get_part().prep_timer;
  }
  catch (...)
  {
  }
  return res;
}
