#ifndef CMDSTAN_ARGUMENTS_ARG_RWM_HPP
#define CMDSTAN_ARGUMENTS_ARG_RWM_HPP

#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

  class arg_rwm: public categorical_argument {
  public:
    arg_rwm() {
      _name = "rwm";
      _description = "Random Walk Metropolis Monte Carlo";
    }
  };

}
#endif
