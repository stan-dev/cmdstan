#ifndef CMDSTAN_ARGUMENTS_VARIATIONAL_RHAT_CUT_HPP
#define CMDSTAN_ARGUMENTS_VARIATIONAL_RHAT_CUT_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_variational_rhat_cut : public real_argument {
 public:
  arg_variational_rhat_cut() : real_argument() {
    _name = "rhat_cut";
    _description = "Rhat threshold to stop preliminary optimization";
    _validity = "1.0 < rhat_cut";
    _default = "1.2";
    _default_value = 1.2;
    _constrained = true;
    _good_value = 1.1;
    _bad_value = 0.9;
    _value = _default_value;
  }
  bool is_valid(double value) { return value > 1.0; }
};
}  // namespace cmdstan
#endif
