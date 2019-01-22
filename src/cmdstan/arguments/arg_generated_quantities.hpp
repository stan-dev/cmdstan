#ifndef CMDSTAN_ARGUMENTS_ARG_GENERATED_QUANTITIES_HPP
#define CMDSTAN_ARGUMENTS_ARG_GENERATED_QUANTITIES_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_generated_quantities_fitted_params.hpp>

namespace cmdstan {

  class arg_generated_quantities: public categorical_argument {
  public:
    arg_generated_quantities() {
      _name = "generated_quantities";
      _description = "Generate quantities of interest";

      _subarguments.push_back(new arg_generated_quantities_fitted_params());
    }
  };

}
#endif
