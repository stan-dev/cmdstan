#ifndef CMDSTAN_ARGUMENTS_ARG_HISTORY_SIZE_HPP
#define CMDSTAN_ARGUMENTS_ARG_HISTORY_SIZE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_history_size : public int_argument {
 public:
  arg_history_size() : int_argument() {
    _name = "history_size";
    _description = "Amount of history to keep for L-BFGS";
    _validity = "0 < history_size";
    _default = "5";
    _default_value = 5;
    _value = _default_value;
  }

  bool is_valid(int value) { return value > 0; }
};

}  // namespace cmdstan
#endif
