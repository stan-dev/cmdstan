#ifndef CMDSTAN_ARGUMENTS_ARG_LAPLACE_DIAG_SHIFT_HPP
#define CMDSTAN_ARGUMENTS_ARG_LAPLACE_DIAG_SHIFT_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

  class arg_laplace_diag_shift : public real_argument {
  public:
    arg_laplace_diag_shift(): real_argument() {
      _name = "laplace_diag_shift";
      _description = "Value added to diagonal of Laplace approximation to avoid singularities";
      _validity = "0 <= laplace_diag_shift";
      _default = "0";
      _default_value = 0.0;
      _constrained = true;
      _good_value = 1e-10;
      _bad_value = -1e-10;
      _value = _default_value;
    }

    bool is_valid(double value) { return value >= 0; }
  };

}
#endif
