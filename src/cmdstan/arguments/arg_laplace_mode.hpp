#ifndef CMDSTAN_ARGUMENTS_ARG_LAPLACE_MODE_HPP
#define CMDSTAN_ARGUMENTS_ARG_LAPLACE_MODE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_laplace_mode : public string_argument {
 public:
  arg_laplace_mode() : string_argument() {
    _name = "mode";
    _description
        = "A specification of a mode on the constrained scale "
          "for all model parameters, either in JSON or CSV format.";
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
