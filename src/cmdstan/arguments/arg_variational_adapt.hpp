#ifndef CMDSTAN_ARGUMENTS_ARG_VARIATIONAL_ADAPT_HPP
#define CMDSTAN_ARGUMENTS_ARG_VARIATIONAL_ADAPT_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_variational_adapt_engaged.hpp>
#include <cmdstan/arguments/arg_variational_adapt_iter.hpp>

namespace cmdstan {

  class arg_variational_adapt: public categorical_argument {
  public:
    arg_variational_adapt() {
      _name = "adapt";
      _description = "Eta Adaptation for Variational Inference";

      _subarguments.push_back(new arg_variational_adapt_engaged());
      _subarguments.push_back(new arg_variational_adapt_iter());
    }
  };

}
#endif
