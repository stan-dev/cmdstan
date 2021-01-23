#ifndef CMDSTAN_ARGUMENTS_VARIATIONAL_ESS_CUT_HPP
#define CMDSTAN_ARGUMENTS_VARIATIONAL_ESS_CUT_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_variational_ess_cut : public real_argument {
 public:
  arg_variational_ess_cut() : real_argument() {
    _name = "ess_cut";
    _description = "ess threshold to set iterate averaging limit";
    _validity = "0 < ess_cut";
    _default = "20";
    _default_value = 20.0;
    _constrained = true;
    _good_value = 20.0;
    _bad_value = -1.0;
    _value = _default_value;
  }
  bool is_valid(double value) { return value > 0.0; }
};
}  // namespace cmdstan
#endif
