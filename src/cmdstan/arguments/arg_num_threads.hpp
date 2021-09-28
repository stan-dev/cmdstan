#ifndef CMDSTAN_ARGUMENTS_ARG_NUM_THREADS_HPP
#define CMDSTAN_ARGUMENTS_ARG_NUM_THREADS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <stan/math/prim/core/init_threadpool_tbb.hpp>

namespace cmdstan {

class arg_num_threads : public int_argument {
 public:
  arg_num_threads() : int_argument() {
    _name = "num_threads";
    _description = std::string(
        "Number of threads available to the program. For full effect, the "
        "model must be compiled with STAN_THREADS=true.");
#ifdef STAN_THREADS
    _validity = "num_threads > 0 || num_threads == -1";
#else
    _validity = "num_threads == 1";
#endif
    _default
        = "1 or the value of the STAN_NUM_THREADS environment variable if set.";
    _default_value = stan::math::internal::get_num_threads();
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
