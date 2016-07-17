#ifndef CMDSTAN_ARGUMENTS_ARG_TEST_GRADIENT_HPP
#define CMDSTAN_ARGUMENTS_ARG_TEST_GRADIENT_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_test_grad_eps.hpp>
#include <cmdstan/arguments/arg_test_grad_err.hpp>

namespace cmdstan {

  class arg_test_gradient: public categorical_argument {
  public:
    arg_test_gradient() {
      _name = "gradient";
      _description = "Check model gradient against finite differences";

      _subarguments.push_back(new arg_test_grad_eps());
      _subarguments.push_back(new arg_test_grad_err());
    }
  };

}
#endif
