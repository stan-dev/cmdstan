#ifndef CMDSTAN_ARGUMENTS_ARG_LAPLACE_MODE_PARAMS_HPP
#define CMDSTAN_ARGUMENTS_ARG_LAPLACE_MODE_PARAMS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_laplace_mode_params : public string_argument {
 public:
  arg_laplace_mode_params() : string_argument() {
    _name = "mode";
    _description
        = "A specification of a mode on the constrained scale "
          "for all model parameters";
    _validity = "Path to existing file";
    _default = "\"\"";
    _default_value = "";
    _constrained = false;
    _good_value = "";
    _value = _default_value;
  }
};

}  // namespace cmdstan

#endif
