/*
 * vm/processor_base.h
 *
 */

#pragma once

#include <stack>
#include <string>
#include <fstream>
using namespace std;

#include "support/execution_stats.h"
#include "support/switchable_output.h"
#include "vm/online_options.h"
#include "vm/instruction.h"
#include "numeric/integer.h"

class ProcessorBase
{
  // Stack
  stack<Integer> stacki;

  ifstream input_file;
  string input_filename;
  size_t input_counter;

protected:
  // Optional argument to tape
  Integer arg;
  vector<Integer> arg_stack;
  vector<int> PC_stack;

  string get_parameterized_filename(int my_num, int thread_num,
      const string& prefix);

public:
  ExecutionStats stats;

  ofstream stdout_redirect_file;

  ProcessorBase();

  void pushi(Integer x) { stacki.push(x); }
  void popi(Integer& x) { x = stacki.top(); stacki.pop(); }

  Integer get_arg() const
    {
      return arg;
    }

  void set_arg(Integer new_arg)
    {
      arg=new_arg;
    }

  void open_input_file(const string& name);
  void open_input_file(int my_num, int thread_num, const string& prefix="");

  template<class T>
  T get_input(bool interactive, const ArgVector::value_type* params);
  template<class T>
  T get_input(istream& is, const string& input_filename,
      const ArgVector::value_type* params);

  void setup_redirection(int my_nu, int thread_num, OnlineOptions& opts,
      SwitchableOutput& out, bool real = true);
};
