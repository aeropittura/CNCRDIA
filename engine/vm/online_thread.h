#pragma once

#include "net/player.h"
#include "numeric/gf2n.h"
#include "numeric/integer.h"
#include "vm/data_files.h"

#include <vector>
using namespace std;

template<class sint, class sgf2n> class Machine;

template<class sint, class sgf2n>
class thread_info
{
  public: 

  int thread_num;
  Names*  Nms;
  typename sgf2n::mac_key_type *alpha2i;
  typename sint::mac_key_type *alphapi;

  Machine<sint, sgf2n>* machine;
  Processor<sint, sgf2n>* processor;

  static void* Main_Func(void *ptr);

  static void purge_preprocessing(const Names& N, int thread_num);

  template<class T>
  static void print_usage(ostream& o, const vector<T>& regs,
      const char* name);

  void Sub_Main_Func();
  void Main_Func_With_Purge();
};
