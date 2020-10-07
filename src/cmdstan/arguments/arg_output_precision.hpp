#ifndef CMDSTAN_ARGUMENTS_ARG_OUTPUT_PRECISION_HPP
#define CMDSTAN_ARGUMENTS_ARG_OUTPUT_PRECISION_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_output_precision : public int_argument {
 public:
  arg_output_precision() : int_argument() {
    _name = "precision";
    _description = "The decimal precision used for the output CSV files.";
    _validity = "integer >= 0 or -1 to use the default precision";
    _default = "-1";
    _default_value = -1;
    _constrained = true;
    _good_value = 8;
    _bad_value = -2;
    _value = _default_value;
  }

  bool is_valid(int value) { return value >= 0 || value == _default_value; }
};

}  // namespace cmdstan
#endif
