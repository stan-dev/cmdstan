#ifndef CMDSTAN_ARGUMENTS_ARG_FIXED_PARAM_HPP
#define CMDSTAN_ARGUMENTS_ARG_FIXED_PARAM_HPP

#include <cmdstan/arguments/unvalued_argument.hpp>

namespace cmdstan {

  class arg_fixed_param: public unvalued_argument {
  public:
    arg_fixed_param() {
      _name = "fixed_param";
      _description = "Fixed Parameter Sampler";
    }
  };

}
#endif
