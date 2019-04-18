#ifndef CMDSTAN_ARGUMENTS_ARG_SWITCHING_E_HPP
#define CMDSTAN_ARGUMENTS_ARG_SWITCHING_E_HPP

#include <cmdstan/arguments/unvalued_argument.hpp>

namespace cmdstan {

  class arg_switching_e: public unvalued_argument {
  public:
    arg_switching_e() {
      _name = "switching_e";
      _description = "Euclidean manifold with sparsity of metric determined at warmup";
    }
  };

}
#endif
