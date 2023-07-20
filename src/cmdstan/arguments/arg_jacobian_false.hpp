#ifndef CMDSTAN_ARGUMENTS_ARG_JACOBIAN_FALSE_HPP
#define CMDSTAN_ARGUMENTS_ARG_JACOBIAN_FALSE_HPP

#include <cmdstan/arguments/arg_jacobian.hpp>

namespace cmdstan {
/**
 * Default for optimization is jacobian=false (legacy behavior).
 */
class arg_jacobian_false : public arg_jacobian {
 public:
  arg_jacobian_false() : arg_jacobian() {
    _default = "0";
    _default_value = false;
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
