#ifndef CMDSTAN_ARGUMENTS_ARG_ITER_HPP
#define CMDSTAN_ARGUMENTS_ARG_ITER_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_iter : public int_argument {
 public:
  arg_iter() : int_argument() {
    _name = "iter";
    _description = "Total number of iterations";
    _validity = "0 < iter";
    _default = "2000";
    _default_value = 2000;
    _constrained = true;
    _good_value = 2.0;
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(int value) { return value > 0; }
};

}  // namespace cmdstan
#endif
