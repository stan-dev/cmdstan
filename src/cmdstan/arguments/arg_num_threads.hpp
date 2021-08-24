#ifndef CMDSTAN_ARGUMENTS_ARG_NUM_THREADS_HPP
#define CMDSTAN_ARGUMENTS_ARG_NUM_THREADS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_num_threads : public int_argument {
 public:
  arg_num_threads() : int_argument() {
    _name = "num_threads";
    _description = std::string("Number of threads available to the program.");
    _validity = "num_threads > 0 || num_threads == -1";
    _default = "1";
    _default_value = 1;
    _good_value = 1.0;
    _bad_value = -2.0;
    _constrained = true;
    _value = _default_value;
  }
#ifdef STAN_THREADS
  bool is_valid(int value) { return value > -2 && value != 0; }
#else
  bool is_valid(int value) { return value == 1; }
#endif
};

}  // namespace cmdstan
#endif
