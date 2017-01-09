#ifndef CMDSTAN_ARGUMENTS_VARIATIONAL_MEANFIELD_HPP
#define CMDSTAN_ARGUMENTS_VARIATIONAL_MEANFIELD_HPP

#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

  class arg_variational_meanfield: public categorical_argument {
  public:
    arg_variational_meanfield() {
      _name = "meanfield";
      _description = "mean-field approximation";
    }
  };
}
#endif
