#ifndef CMDSTAN_ARGUMENTS_ARG_HMC_HPP
#define CMDSTAN_ARGUMENTS_ARG_HMC_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_engine.hpp>
#include <cmdstan/arguments/arg_metric.hpp>
#include <cmdstan/arguments/arg_metric_file.hpp>
#include <cmdstan/arguments/arg_stepsize.hpp>
#include <cmdstan/arguments/arg_stepsize_jitter.hpp>

namespace cmdstan {

  class arg_hmc: public categorical_argument {
  public:
    arg_hmc() {
      _name = "hmc";
      _description = "Hamiltonian Monte Carlo";

      _subarguments.push_back(new arg_engine());
      _subarguments.push_back(new arg_metric());
      _subarguments.push_back(new arg_metric_file());
      _subarguments.push_back(new arg_stepsize());
      _subarguments.push_back(new arg_stepsize_jitter());
    }
  };

}
#endif
