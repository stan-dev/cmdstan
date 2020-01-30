#ifndef CMDSTAN_ARGUMENTS_ARG_ADAPT_NUM_CROSS_CHAIN_HPP
#define CMDSTAN_ARGUMENTS_ARG_ADAPT_NUM_CROSS_CHAIN_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

  class arg_adapt_num_cross_chains: public u_int_argument {
  public:
    arg_adapt_num_cross_chains(): u_int_argument() {
      _name = "num_cross_chains";
      _description = "Number of chains in cross-chain warmup iterations";
      _default = "4";
      _default_value = 4;
      _value = _default_value;
    }
  };

}
#endif
