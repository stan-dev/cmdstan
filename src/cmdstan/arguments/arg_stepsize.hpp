#ifndef CMDSTAN_ARGUMENTS_ARG_STEPSIZE_HPP
#define CMDSTAN_ARGUMENTS_ARG_STEPSIZE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_stepsize : public real_argument {
 public:
  arg_stepsize() : real_argument() {
    _name = "stepsize";
    _description = "Step size for discrete evolution";
    _validity = "0 < stepsize";
    _default = "1";
    _default_value = 1.0;
    _constrained = true;
    _good_value = 2.0;
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(double value) { return value > 0; }
};

}  // namespace cmdstan
#endif
