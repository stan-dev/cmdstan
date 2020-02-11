#ifndef CMDSTAN_ARGUMENTS_ARG_ADAPT_CROSS_CHAIN_ESS_HPP
#define CMDSTAN_ARGUMENTS_ARG_ADAPT_CROSS_CHAIN_ESS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

  class arg_adapt_cross_chain_ess: public u_int_argument {
  public:
    arg_adapt_cross_chain_ess(): u_int_argument() {
      _name = "cross_chain_ess";
      _description = "Target ESS for cross-chain warmup";
      _default = "200";
      _default_value = 200;
      _value = _default_value;
    }
  };

}
#endif
