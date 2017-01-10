#ifndef CMDSTAN_ARGUMENTS_ARG_ADAPT_HPP
#define CMDSTAN_ARGUMENTS_ARG_ADAPT_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_adapt_engaged.hpp>
#include <cmdstan/arguments/arg_adapt_gamma.hpp>
#include <cmdstan/arguments/arg_adapt_delta.hpp>
#include <cmdstan/arguments/arg_adapt_kappa.hpp>
#include <cmdstan/arguments/arg_adapt_t0.hpp>
#include <cmdstan/arguments/arg_adapt_init_buffer.hpp>
#include <cmdstan/arguments/arg_adapt_term_buffer.hpp>
#include <cmdstan/arguments/arg_adapt_window.hpp>

namespace cmdstan {
  class arg_adapt: public categorical_argument {
  public:
    arg_adapt() {
      _name = "adapt";
      _description = "Warmup Adaptation";

      _subarguments.push_back(new arg_adapt_engaged());
      _subarguments.push_back(new arg_adapt_gamma());
      _subarguments.push_back(new arg_adapt_delta());
      _subarguments.push_back(new arg_adapt_kappa());
      _subarguments.push_back(new arg_adapt_t0());
      _subarguments.push_back(new arg_adapt_init_buffer());
      _subarguments.push_back(new arg_adapt_term_buffer());
      _subarguments.push_back(new arg_adapt_window());
    }
  };

}
#endif
