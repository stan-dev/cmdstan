#ifndef CMDSTAN_ARGUMENTS_ARG_LAPLACE_DRAWS_HPP
#define CMDSTAN_ARGUMENTS_ARG_LAPLACE_DRAWS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_laplace_draws : public int_argument {
 public:
  arg_laplace_draws() : int_argument() {
    _name = "draws";
    _description = "Number of draws from the laplace approximation";
    _validity = "0 <= draws";
    _default = "1000";
    _default_value = 1000;
    _constrained = true;
    _good_value = 2;
    _bad_value = -2;
    _value = _default_value;
  }

  bool is_valid(int value) { return value >= 0; }
};

}  // namespace cmdstan
#endif
