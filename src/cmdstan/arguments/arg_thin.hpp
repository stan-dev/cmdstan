#ifndef CMDSTAN_ARGUMENTS_ARG_THIN_HPP
#define CMDSTAN_ARGUMENTS_ARG_THIN_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_thin : public int_argument {
 public:
  arg_thin() : int_argument() {
    _name = "thin";
    _description = "Period between saved samples";
    _validity = "0 < thin";
    _default = "1";
    _default_value = 1;
    _value = _default_value;
  }

  bool is_valid(int value) { return value > 0; }
};

}  // namespace cmdstan
#endif
