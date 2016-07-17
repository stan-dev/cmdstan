#ifndef CMDSTAN_ARGUMENTS_ARG_UNIT_E_HPP
#define CMDSTAN_ARGUMENTS_ARG_UNIT_E_HPP

#include <cmdstan/arguments/unvalued_argument.hpp>

namespace cmdstan {

  class arg_unit_e: public unvalued_argument {
  public:
    arg_unit_e() {
      _name = "unit_e";
      _description = "Euclidean manifold with unit metric";
    }
  };

}
#endif
