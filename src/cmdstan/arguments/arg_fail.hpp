#ifndef CMDSTAN_ARGUMENTS_ARG_FAIL_HPP
#define CMDSTAN_ARGUMENTS_ARG_FAIL_HPP

#include <cmdstan/arguments/unvalued_argument.hpp>

namespace cmdstan {

  class arg_fail: public unvalued_argument {
  public:
    arg_fail() {
      _name = "fail";
      _description = "Dummy argument to induce failures for testing";
    }
  };

}
#endif
