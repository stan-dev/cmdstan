#ifndef CMDSTAN_ARGUMENTS_ARG_GENERATE_QUANTITIES_HPP
#define CMDSTAN_ARGUMENTS_ARG_GENERATE_QUANTITIES_HPP

#include <cmdstan/arguments/arg_generate_quantities_fitted_params.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_generate_quantities : public categorical_argument {
 public:
  arg_generate_quantities() {
    _name = "generate_quantities";
    _description = "Generate quantities of interest";

    _subarguments.push_back(new arg_generate_quantities_fitted_params());
  }
};

}  // namespace cmdstan
#endif
