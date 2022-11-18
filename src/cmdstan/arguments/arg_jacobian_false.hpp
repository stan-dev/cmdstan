#ifndef CMDSTAN_ARGUMENTS_ARG_JACOBIAN_FALSE_HPP
#define CMDSTAN_ARGUMENTS_ARG_JACOBIAN_FALSE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {
/**
 * Argument to indicate whether calculated log-probability density should
 * include the log absolute Jacobian determinant of inverse parameter transforms
 * For optimization, default should be false.
 */
class arg_jacobian_false : public bool_argument {
 public:
  arg_jacobian_false() : bool_argument() {
    _name = "jacobian";
    _description
        = "When true, include change-of-variables adjustment"
          "for constraining parameter transforms";
    _validity = "[0, 1]";
    _default = "0";
    _default_value = false;
    _constrained = false;
    _good_value = 1;
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
