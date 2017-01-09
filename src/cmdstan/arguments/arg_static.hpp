#ifndef CMDSTAN_ARGUMENTS_ARG_STATIC_HPP
#define CMDSTAN_ARGUMENTS_ARG_STATIC_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_int_time.hpp>

namespace cmdstan {

  class arg_static: public categorical_argument {
  public:
    arg_static() {
      _name = "static";
      _description = "Static integration time";

      _subarguments.push_back(new arg_int_time());
    }
  };

}
#endif
