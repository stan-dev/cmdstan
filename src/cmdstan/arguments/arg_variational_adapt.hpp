#ifndef CMDSTAN_ARGUMENTS_ARG_VARIATIONAL_ADAPT_HPP
#define CMDSTAN_ARGUMENTS_ARG_VARIATIONAL_ADAPT_HPP

#include <cmdstan/arguments/arg_single_bool.hpp>
#include <cmdstan/arguments/arg_single_int_pos.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>
#include <stan/services/experimental/advi/defaults.hpp>

namespace cmdstan {

using stan::services::experimental::advi::adapt_engaged;
using stan::services::experimental::advi::adapt_iterations;

class arg_variational_adapt : public categorical_argument {
 public:
  arg_variational_adapt() {
    _name = "adapt";
    _description = "Eta Adaptation for Variational Inference";

    _subarguments.push_back(
        new arg_single_bool("engaged", adapt_engaged::description().c_str(),
                            adapt_engaged::default_value()));
    _subarguments.push_back(
        new arg_single_int_pos("iter", adapt_iterations::description().c_str(),
                               adapt_iterations::default_value()));
  }
};

}  // namespace cmdstan
#endif
