#ifndef CMDSTAN_ARGUMENTS_ARG_OUTPUT_SIG_FIGS_HPP
#define CMDSTAN_ARGUMENTS_ARG_OUTPUT_SIG_FIGS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_output_sig_figs : public int_argument {
 public:
  arg_output_sig_figs() : int_argument() {
    _name = "sig_figs";
    _description
        = "The number of significant figures used for the output CSV files. "
          "Stan recommends at least 8 (the default) if you will be using the "
          "output as part of a future input (e.g., for standalone generated "
          "quantities or as an initialization). If -1 is supplied, Stan will "
          "use the current operating system's default precision (This "
          "typically be smaller than Stan's default; on many systems it is 6 "
          "digits).";
    _validity = "0 <= sig_figs <= 18 || sig_figs == -1";
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
