#ifndef CMDSTAN_ARGUMENTS_ARG_ADAPT_CROSS_CHAIN_WINDOW_HPP
#define CMDSTAN_ARGUMENTS_ARG_ADAPT_CROSS_CHAIN_WINDOW_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

  class arg_adapt_cross_chain_window: public u_int_argument {
  public:
    arg_adapt_cross_chain_window(): u_int_argument() {
      _name = "cross_chain_window";
      _description = "Window size for cross-chain warmup";
      _default = "100";
      _default_value = 100;
      _value = _default_value;
    }
  };

}
#endif
