#ifndef STAN_VARIATIONAL_EVAL_WINDOW_HPP
#define STAN_VARIATIONAL_EVAL_WINDOW_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_variational_eval_window : public int_argument {
 public:
  arg_variational_eval_window() : int_argument() {
    _name = "eval_window";
    _description = "Iterations in each evaluation window";
    _validity = "0 < eval_window";
    _default = "100";
    _default_value = 100;
    _constrained = true;
    _good_value = 2.0;
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(int value) { return value > 0; }
};

}  // namespace cmdstan
#endif
