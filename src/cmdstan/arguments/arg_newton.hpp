#ifndef CMDSTAN_ARGUMENTS_ARG_NEWTON_HPP
#define CMDSTAN_ARGUMENTS_ARG_NEWTON_HPP

#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

  class arg_newton: public categorical_argument {
  public:
    arg_newton() {
      _name = "newton";
      _description = "Newton's method";
    }
  };

}
#endif
