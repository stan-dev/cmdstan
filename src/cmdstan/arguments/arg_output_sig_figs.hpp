#ifndef CMDSTAN_ARGUMENTS_ARG_OUTPUT_SIG_FIGS_HPP
#define CMDSTAN_ARGUMENTS_ARG_OUTPUT_SIG_FIGS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_output_sig_figs : public int_argument {
 public:
  arg_output_sig_figs() : int_argument() {
    _name = "sig_figs";
    _description
        = "The number of significant figures used for the output CSV files.";
    _validity
        = "0 <= integer <= 18, or -1 to use the system library's default "
          "number of significant figures (usually 6).";
    _default = "8";
    _default_value = 8;
    _value = _default_value;
  }

  bool is_valid(int value) {
    return (value >= 0 && value <= 18) || value == -1;
  }
};

}  // namespace cmdstan
#endif
