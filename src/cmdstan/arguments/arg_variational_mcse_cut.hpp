#ifndef CMDSTAN_ARGUMENTS_VARIATIONAL_MCSE_CUT_HPP
#define CMDSTAN_ARGUMENTS_VARIATIONAL_MCSE_CUT_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_variational_mcse_cut : public real_argument {
 public:
  arg_variational_mcse_cut() : real_argument() {
    _name = "mcse_cut";
    _description = "mcse threshold to set iterate averaging limit";
    _validity = "0 < mcse_cut";
    _default = "0.02";
    _default_value = 0.02;
    _constrained = true;
    _good_value = 1.0;
    _bad_value = -1.0;
    _value = _default_value;
  }
  bool is_valid(double value) { return value > 0.0; }
};
}  // namespace cmdstan
#endif
