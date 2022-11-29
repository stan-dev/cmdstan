#ifndef CMDSTAN_ARGUMENTS_ARG_LAPLACE_HPP
#define CMDSTAN_ARGUMENTS_ARG_LAPLACE_HPP

#include <cmdstan/arguments/arg_laplace_draws.hpp>
#include <cmdstan/arguments/arg_jacobian.hpp>
#include <cmdstan/arguments/arg_laplace_mode.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_laplace : public categorical_argument {
 public:
  arg_laplace() {
    _name = "laplace";
    _description = "Sample from a Laplace approximation";

    _subarguments.push_back(new arg_laplace_mode());
    _subarguments.push_back(new arg_jacobian());
    _subarguments.push_back(new arg_laplace_draws());
  }
};

}  // namespace cmdstan
#endif
