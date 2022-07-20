#ifndef CMDSTAN_ARGUMENTS_ARG_LOG_PROB_HPP
#define CMDSTAN_ARGUMENTS_ARG_LOG_PROB_HPP

#include <cmdstan/arguments/arg_log_prob_unconstrained_params.hpp>
#include <cmdstan/arguments/arg_log_prob_constrained_params.hpp>
#include <cmdstan/arguments/arg_log_prob_jacobian_adjust.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_log_prob : public categorical_argument {
 public:
  arg_log_prob() {
    _name = "log_prob";
    _description
        = "Return the log density up to a constant and its gradients, "
          "given supplied parameters";

    _subarguments.emplace_back(
        std::move(new arg_log_prob_unconstrained_params()));
    _subarguments.emplace_back(
        std::move(new arg_log_prob_constrained_params()));
    _subarguments.emplace_back(std::move(new arg_log_prob_jacobian_adjust()));
  }
};

}  // namespace cmdstan
#endif
