#ifndef CMDSTAN_ARGUMENTS_ARG_DENSE_E_HPP
#define CMDSTAN_ARGUMENTS_ARG_DENSE_E_HPP

#include <cmdstan/arguments/unvalued_argument.hpp>

namespace cmdstan {

  class arg_dense_e: public unvalued_argument {
  public:
    arg_dense_e() {
      _name = "dense_e";
      _description = "Euclidean manifold with dense metric";
    }
  };

}
#endif
