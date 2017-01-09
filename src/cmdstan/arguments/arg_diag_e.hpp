#ifndef CMDSTAN_ARGUMENTS_ARG_DIAG_E_HPP
#define CMDSTAN_ARGUMENTS_ARG_DIAG_E_HPP

#include <cmdstan/arguments/unvalued_argument.hpp>

namespace cmdstan {

  class arg_diag_e: public unvalued_argument {
  public:
    arg_diag_e() {
      _name = "diag_e";
      _description = "Euclidean manifold with diag metric";
    }
  };

}
#endif
