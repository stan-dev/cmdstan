#ifndef CMDSTAN_ARGUMENTS_ARG_REFRESH_HPP
#define CMDSTAN_ARGUMENTS_ARG_REFRESH_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_refresh : public int_argument {
 public:
  arg_refresh() : int_argument() {
    _name = "refresh";
    _description = "Number of iterations between screen updates";
    _validity = "0 <= refresh";
    _default = "100";
    _default_value = 100;
    _value = _default_value;
  }

  bool is_valid(int value) { return value >= 0; }
};

}  // namespace cmdstan
#endif
