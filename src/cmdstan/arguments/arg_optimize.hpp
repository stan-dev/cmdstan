#ifndef CMDSTAN_ARGUMENTS_ARG_OPTIMIZE_HPP
#define CMDSTAN_ARGUMENTS_ARG_OPTIMIZE_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_optimize_algo.hpp>
#include <cmdstan/arguments/arg_iter.hpp>
#include <cmdstan/arguments/arg_save_iterations.hpp>

namespace cmdstan {

  class arg_optimize: public categorical_argument {
  public:
    arg_optimize() {
      _name = "optimize";
      _description = "Point estimation";

      _subarguments.push_back(new arg_optimize_algo());
      _subarguments.push_back(new arg_iter());
      _subarguments.push_back(new arg_save_iterations());
    }
  };

}
#endif
