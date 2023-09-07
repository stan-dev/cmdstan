#ifndef CMDSTAN_ARGUMENTS_ARG_ID_HPP
#define CMDSTAN_ARGUMENTS_ARG_ID_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_id : public int_argument {
 public:
  arg_id() : int_argument() {
    _name = "id";
    _description = "Unique process identifier";
    _validity = "id >= 0";
    _default = "1";
    _default_value = 1;
    _value = _default_value;
  }

  bool is_valid(int value) { return value >= 0; }
};

}  // namespace cmdstan
#endif
