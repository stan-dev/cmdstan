#ifndef CMDSTAN_ARGUMENTS_ARG_LOG_PROB_JACOBIAN_ADJUST_HPP
#define CMDSTAN_ARGUMENTS_ARG_LOG_PROB_JACOBIAN_ADJUST_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {
/**
 * Argument to indicate whether calculated log-probability density should
 * include the log absolute Jacobian determinant of inverse parameter transforms
 */
class arg_log_prob_jacobian_adjust : public bool_argument {
 public:
  arg_log_prob_jacobian_adjust() : bool_argument() {
    _name = "jacobian_adjust";
    _description = "Apply jacobian adjustments to log-probability?";
    _validity = "[0, 1]";
    _default = "1";
    _default_value = true;
    _constrained = false;
    _good_value = 1;
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
