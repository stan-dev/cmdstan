#ifndef CMDSTAN_ARGUMENTS_ARG_SOFTABS_ALPHA_HPP
#define CMDSTAN_ARGUMENTS_ARG_SOFTABS_ALPHA_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_softabs_alpha : public real_argument {
 public:
  arg_softabs_alpha() : real_argument() {
    _name = "alpha";
    _description = "Softabs regularization parameter";
    _validity = "0 < alpha";
    _default = "1.0";
    _default_value = 1.0;
    _constrained = true;
    _good_value = 1.0;
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(double value) { return 0 < value; }
};

}  // namespace cmdstan
#endif
