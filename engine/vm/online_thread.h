#pragma once

#include "net/player.h"
#include "numeric/gf2n.h"
#include "numeric/integer.h"
#include "vm/data_files.h"

#include <vector>
using namespace std;

template<class Sint, class Sgf2n> class Machine;

template<class Sint, class Sgf2n>
class thread_info
{
  public: 

  int thread_num;
  Names*  nms;
  typename Sgf2n::MacKeyType *alpha2i;
  typename Sint::MacKeyType *alphapi;

  Machine<Sint, Sgf2n>* machine;
  Processor<Sint, Sgf2n>* processor;

  static void* main_func(void *ptr);

  static void purge_preprocessing(const Names& N, int thread_num);

  template<class T>
  static void print_usage(ostream& o, const vector<T>& regs,
      const char* name);

  void sub_main_func();
  void main_func_with_purge();
};
