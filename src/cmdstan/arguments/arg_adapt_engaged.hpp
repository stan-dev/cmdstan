#ifndef CMDSTAN_ARGUMENTS_ARG_ADAPT_ENGAGED_HPP
#define CMDSTAN_ARGUMENTS_ARG_ADAPT_ENGAGED_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_adapt_engaged : public bool_argument {
 public:
  arg_adapt_engaged() : bool_argument() {
    _name = "engaged";
    _description = "Adaptation engaged?";
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
