#ifndef CMDSTAN_ARGUMENTS_ARG_DIAGNOSE_HPP
#define CMDSTAN_ARGUMENTS_ARG_DIAGNOSE_HPP

#include <cmdstan/arguments/arg_test.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_diagnose : public categorical_argument {
 public:
  arg_diagnose() {
    _name = "diagnose";
    _description = "Model diagnostics";

    _subarguments.push_back(new arg_test());
  }
};

}  // namespace cmdstan
#endif
