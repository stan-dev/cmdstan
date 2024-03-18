#ifndef CMDSTAN_ARGUMENTS_ARG_METHOD_HPP
#define CMDSTAN_ARGUMENTS_ARG_METHOD_HPP

#include <cmdstan/arguments/arg_diagnose.hpp>
#include <cmdstan/arguments/arg_generate_quantities.hpp>
#include <cmdstan/arguments/arg_optimize.hpp>
#include <cmdstan/arguments/arg_pathfinder.hpp>
#include <cmdstan/arguments/arg_sample.hpp>
#include <cmdstan/arguments/arg_variational.hpp>
#include <cmdstan/arguments/arg_log_prob.hpp>
#include <cmdstan/arguments/arg_laplace.hpp>
#include <cmdstan/arguments/list_argument.hpp>

namespace cmdstan {

class arg_method : public list_argument {
 public:
  arg_method() {
    _name = "method";
    _description = "Analysis method (Note that method= is optional)";

    _values.push_back(new arg_sample());
    _values.push_back(new arg_optimize());
    _values.push_back(new arg_variational());
    _values.push_back(new arg_diagnose());
    _values.push_back(new arg_generate_quantities());
    _values.push_back(new arg_pathfinder());
    _values.push_back(new arg_log_prob());
    _values.push_back(new arg_laplace());

    _default_cursor = 0;
    _cursor = _default_cursor;
  }
};

}  // namespace cmdstan
#endif
