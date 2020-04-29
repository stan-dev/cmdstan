#ifndef CMDSTAN_ARGUMENTS_ARG_SAVE_WARMUP_HPP
#define CMDSTAN_ARGUMENTS_ARG_SAVE_WARMUP_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_save_warmup : public bool_argument {
 public:
  arg_save_warmup() : bool_argument() {
    _name = "save_warmup";
    _description = "Stream warmup samples to output?";
    _validity = "[0, 1]";
    _default = "0";
    _default_value = false;
    _constrained = false;
    _good_value = 1;
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
