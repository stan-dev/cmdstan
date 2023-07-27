#ifndef CMDSTAN_ARGUMENTS_ARG_VARIATIONAL_ADAPT_ITER_HPP
#define CMDSTAN_ARGUMENTS_ARG_VARIATIONAL_ADAPT_ITER_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <stan/services/experimental/advi/defaults.hpp>
#include <string>

namespace cmdstan {

using stan::services::experimental::advi::adapt_iterations;

class arg_variational_adapt_iter : public int_argument {
 public:
  arg_variational_adapt_iter() : int_argument() {
    _name = "iter";
    _description = adapt_iterations::description();
    _validity = "0 < iter";
    _default = std::to_string(adapt_iterations::default_value());
    _default_value = adapt_iterations::default_value();
    _constrained = true;
    _good_value = adapt_iterations::default_value();
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(int value) { return value > 0; }
};

}  // namespace cmdstan
#endif
