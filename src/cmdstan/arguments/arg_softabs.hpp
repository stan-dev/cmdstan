#ifndef CMDSTAN_ARGUMENTS_ARG_SOFTABS_HPP
#define CMDSTAN_ARGUMENTS_ARG_SOFTABS_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_softabs_alpha.hpp>

namespace cmdstan {

  class arg_softabs: public categorical_argument {
  public:
    arg_softabs() {
      _name = "softabs";
      _description = "Riemannian manifold with SoftAbs metric";

      _subarguments.push_back(new arg_softabs_alpha());
    }
  };

}
#endif
