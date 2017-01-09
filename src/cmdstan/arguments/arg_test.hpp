#ifndef CMDSTAN_ARGUMENTS_ARG_TEST_HPP
#define CMDSTAN_ARGUMENTS_ARG_TEST_HPP

#include <cmdstan/arguments/list_argument.hpp>
#include <cmdstan/arguments/arg_test_gradient.hpp>

namespace cmdstan {

  class arg_test: public list_argument {
  public:
    arg_test() {
      _name = "test";
      _description = "Diagnostic test";

      _values.push_back(new arg_test_gradient());

      _default_cursor = 0;
      _cursor = _default_cursor;
    }
  };

}
#endif
