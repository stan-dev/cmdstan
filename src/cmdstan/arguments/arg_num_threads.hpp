#ifndef CMDSTAN_ARGUMENTS_ARG_NUM_THREADS_HPP
#define CMDSTAN_ARGUMENTS_ARG_NUM_THREADS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <stan/math/prim/core/init_threadpool_tbb.hpp>

namespace cmdstan {

class arg_num_threads : public int_argument {
 public:
  arg_num_threads() : int_argument() {
    _name = "num_threads";
#ifdef STAN_THREADS
    _description = std::string("Number of threads available to the program.");
    _validity = "num_threads > 0 || num_threads == -1";
    _default
        = "1 or the value of the STAN_NUM_THREADS environment variable if set.";
#else
    _description = std::string(
        "Number of threads available to the program. To use this "
        "argument, re-compile this model with STAN_THREADS=true.");
    _validity = "num_threads == 1";
    _default = "1";
#endif
    _default_value = stan::math::internal::get_num_threads();
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
