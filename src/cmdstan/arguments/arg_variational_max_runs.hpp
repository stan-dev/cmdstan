#ifndef STAN_VARIATIONAL_MAX_RUNS_HPP
#define STAN_VARIATIONAL_MAX_RUNS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_variational_max_runs : public int_argument {
 public:
  arg_variational_max_runs() : int_argument() {
    _name = "max_runs";
    _description = "Another max iterations?";
    _validity = "0 < max_runs";
    _default
        = "1000";
    _default_value = 1000;
    _constrained = true;
    _good_value = 2.0;
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(int value) { return value > 0; }
};

}  // namespace cmdstan
#endif
