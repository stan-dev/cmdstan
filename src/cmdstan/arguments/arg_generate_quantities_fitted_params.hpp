#ifndef CMDSTAN_ARGUMENTS_ARG_GENERATE_QUANTITIES_FITTED_PARAMS_HPP
#define CMDSTAN_ARGUMENTS_ARG_GENERATE_QUANTITIES_FITTED_PARAMS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_generate_quantities_fitted_params : public string_argument {
 public:
  arg_generate_quantities_fitted_params() : string_argument() {
    _name = "fitted_params";
    _description
        = "Input file of sample of fitted parameter values for model "
          "conditioned on data";
    _validity = "Path to existing file";
    _default = "\"\"";
    _default_value = "";
    _value = _default_value;
  }
};

}  // namespace cmdstan

#endif
