#ifndef CMDSTAN_ARGUMENTS_ARG_GENERATED_QUANTITIES_FITTED_PARAMS_HPP
#define CMDSTAN_ARGUMENTS_ARG_GENERATED_QUANTITIES_FITTED_PARAMS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

  class arg_generated_quantities_fitted_params: public string_argument {
  public:
    arg_generated_quantities_fitted_params(): string_argument() {
      _name = "fitted_params";
      _description = "Input file of sample of fitted parameter values for model conditioned on data";
      _validity = "Path to existing file";
      _default = "\"\"";
      _default_value = "";
      _constrained = false;
      _good_value = "";
      _value = _default_value;
    }
  };

}

#endif
