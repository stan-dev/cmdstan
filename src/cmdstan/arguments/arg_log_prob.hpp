#ifndef CMDSTAN_ARGUMENTS_ARG_LOG_PROB_HPP
#define CMDSTAN_ARGUMENTS_ARG_LOG_PROB_HPP

#include <cmdstan/arguments/arg_log_prob_unconstrained_params.hpp>
#include <cmdstan/arguments/arg_log_prob_constrained_params.hpp>
#include <cmdstan/arguments/arg_single_bool.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

/**
 * Argument used for calling the log_prob method to calculate the log density
 * and its gradients with respect to a user-provided set of parameter values
 * on the constrained and/or unconstrained scale
 */
class arg_log_prob : public categorical_argument {
 public:
  arg_log_prob() {
    _name = "log_prob";
    _description
        = "Return the log density up to a constant and its gradients, "
          "given supplied parameters";

    _subarguments.push_back(new arg_log_prob_unconstrained_params());
    _subarguments.push_back(new arg_log_prob_constrained_params());
    _subarguments.push_back(
        new arg_single_bool("jacobian",
                            "When true, include change-of-variables adjustment"
                            " for constraining parameter transforms",
                            true));
  }
};

}  // namespace cmdstan
#endif
