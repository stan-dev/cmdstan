#ifndef CMDSTAN_ARGUMENTS_ARG_AUTO_E_HPP
#define CMDSTAN_ARGUMENTS_ARG_AUTO_E_HPP

#include <cmdstan/arguments/unvalued_argument.hpp>

namespace cmdstan {

  class arg_auto_e: public unvalued_argument {
  public:
    arg_auto_e() {
      _name = "auto_e";
      _description = "Euclidean manifold that chooses between dense/diagonal metric at warmup";
    }
  };

}
#endif
