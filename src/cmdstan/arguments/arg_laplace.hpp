#ifndef CMDSTAN_ARGUMENTS_ARG_LAPLACE_HPP
#define CMDSTAN_ARGUMENTS_ARG_LAPLACE_HPP

#include <cmdstan/arguments/arg_single_bool.hpp>
#include <cmdstan/arguments/arg_single_int_nonneg.hpp>
#include <cmdstan/arguments/arg_single_string.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_laplace : public categorical_argument {
 public:
  arg_laplace() {
    _name = "laplace";
    _description = "Sample from a Laplace approximation";

    _subarguments.push_back(new arg_single_string(
        "mode",
        "A specification of a mode on the constrained scale "
        "for all model parameters, either in JSON or CSV format.",
        ""));
    _subarguments.push_back(
        new arg_single_bool("jacobian",
                            "When true, include change-of-variables adjustment "
                            "for constraining parameter transforms.",
                            true));
    _subarguments.push_back(new arg_single_int_nonneg(
        "draws", "Number of draws from the laplace approximation", 1000));
    _subarguments.push_back(new arg_single_bool(
        "calculate_lp",
        "If true, calculate the log probability of the model at each draw.",
        true));
  }
};

}  // namespace cmdstan
#endif
