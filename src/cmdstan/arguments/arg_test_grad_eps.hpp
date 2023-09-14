#ifndef CMDSTAN_ARGUMENTS_ARG_TEST_GRAD_EPS_HPP
#define CMDSTAN_ARGUMENTS_ARG_TEST_GRAD_EPS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_test_grad_eps : public real_argument {
 public:
  arg_test_grad_eps() : real_argument() {
    _name = "epsilon";
    _description = "Finite difference step size";
    _validity = "0 < epsilon";
    _default = "1e-6";
    _default_value = 1e-6;
    _value = _default_value;
  }

  bool is_valid(double value) { return value > 0; }
};

}  // namespace cmdstan
#endif
