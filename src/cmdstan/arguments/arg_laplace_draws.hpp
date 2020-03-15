#ifndef CMDSTAN_ARGUMENTS_ARG_LAPLACE_DRAWS_HPP
#define CMDSTAN_ARGUMENTS_ARG_LAPLACE_DRAWS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

  class arg_laplace_draws : public int_argument {
  public:
    arg_laplace_draws(): int_argument() {
      _name = "laplace_draws";
      _description = "Number of draws from Laplace approximation to posterior";
      _validity = "0 <= laplace_draws";
      _default = "0";
      _default_value = 0;
      _constrained = true;
      _good_value = 2;
      _bad_value = -1;
      _value = _default_value;
    }

    bool is_valid(int value) { return value >= 0; }
  };

}
#endif
