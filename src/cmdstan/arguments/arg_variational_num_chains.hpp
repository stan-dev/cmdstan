#ifndef STAN_VARIATIONAL_NUM_CHAINS_HPP
#define STAN_VARIATIONAL_NUM_CHAINS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_variational_num_chains : public int_argument {
 public:
  arg_variational_num_chains() : int_argument() {
    _name = "num_chains";
    _description = "Iterations in each evaluation window";
    _validity = "0 < num_chains";
    _default = "4";
    _default_value = 4;
    _constrained = true;
    _good_value = 2.0;
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(int value) { return value > 0; }
};

}  // namespace cmdstan
#endif
