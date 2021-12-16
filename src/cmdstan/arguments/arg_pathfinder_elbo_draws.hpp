#ifndef CMDSTAN_ARGUMENTS_ARG_ELBO_DRAWS_HPP
#define CMDSTAN_ARGUMENTS_ARG_ELBO_DRAWS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_num_elbo_draws : public int_argument {
 public:
  arg_num_elbo_draws() : int_argument() {
    _name = "num_elbo_draws";
    _description = "todo";
    _default = "25";
    _default_value = 25;
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
