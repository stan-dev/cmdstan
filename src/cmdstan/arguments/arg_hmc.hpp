#ifndef CMDSTAN_ARGUMENTS_ARG_HMC_HPP
#define CMDSTAN_ARGUMENTS_ARG_HMC_HPP

#include <cmdstan/arguments/arg_engine.hpp>
#include <cmdstan/arguments/arg_metric.hpp>
#include <cmdstan/arguments/arg_single_real_bounded.hpp>
#include <cmdstan/arguments/arg_single_real_pos.hpp>
#include <cmdstan/arguments/arg_single_string.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_hmc : public categorical_argument {
 public:
  arg_hmc() {
    _name = "hmc";
    _description = "Hamiltonian Monte Carlo";

    _subarguments.push_back(new arg_engine());
    _subarguments.push_back(new arg_metric());
    _subarguments.push_back(new arg_single_string(
        "metric_file", "Input file with precomputed Euclidean metric", ""));
    _subarguments.push_back(new arg_single_real_pos(
        "stepsize", "Step size for discrete evolution", 1));
    _subarguments.push_back(new arg_single_real_bounded(
        "stepsize_jitter",
        "Uniformly random jitter of the stepsize, in percent", 0.0, 0.0, 1.0));
  }
};

}  // namespace cmdstan
#endif
