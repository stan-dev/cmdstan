#ifndef CMDSTAN_ARGUMENTS_ARG_LBFGS_HPP
#define CMDSTAN_ARGUMENTS_ARG_LBFGS_HPP

#include <cmdstan/arguments/arg_bfgs.hpp>
#include <cmdstan/arguments/arg_history_size.hpp>

namespace cmdstan {

  class arg_lbfgs: public arg_bfgs {
  public:
    arg_lbfgs() {
      _name = "lbfgs";
      _description = "LBFGS with linesearch";

      _subarguments.push_back(new arg_history_size());
    }
  };

}
#endif
