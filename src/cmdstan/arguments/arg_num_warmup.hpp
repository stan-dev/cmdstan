#ifndef CMDSTAN_ARGUMENTS_ARG_NUM_WARMUP_HPP
#define CMDSTAN_ARGUMENTS_ARG_NUM_WARMUP_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_num_warmup : public int_argument {
 public:
  arg_num_warmup() : int_argument() {
    _name = "num_warmup";
    _description = "Number of warmup iterations";
    _validity = "0 <= warmup";
    _default = "1000";
    _default_value = 1000;
    _constrained = true;
    _good_value = 2.0;
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(int value) { return value >= 0; }
};

}  // namespace cmdstan
#endif
