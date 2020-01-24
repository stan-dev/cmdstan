#ifndef CMDSTAN_ARGUMENTS_ARG_NUM_CROSS_CHAIN_HPP
#define CMDSTAN_ARGUMENTS_ARG_NUM_CROSS_CHAIN_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

  class arg_num_cross_chains: public int_argument {
  public:
    arg_num_cross_chains(): int_argument() {
      _name = "num_cross_chains";
      _description = "Number of chains in cross-chain warmup iterations";
      _validity = "4 <= chains";
      _default = "4";
      _default_value = 4;
      _constrained = true;
      _good_value = 2.0;
      _bad_value = -1.0;
      _value = _default_value;
    }

    bool is_valid(int value) { return value >= 4; }
  };

}
#endif
