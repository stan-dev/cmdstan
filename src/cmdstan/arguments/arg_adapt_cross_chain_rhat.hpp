#ifndef CMDSTAN_ARGUMENTS_ARG_ADAPT_CROSS_CHAIN_RHAT_HPP
#define CMDSTAN_ARGUMENTS_ARG_ADAPT_CROSS_CHAIN_RHAT_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

  class arg_adapt_cross_chain_rhat: public real_argument {
  public:
    arg_adapt_cross_chain_rhat(): real_argument() {
      _name = "cross_chain_rhat";
      _description = "Target Rhat for cross-chain warmup";
      _validity = "0.8 < cross_chain_rhat";
      _default = "1.05";
      _default_value = 1.05;
      _constrained = true;
      _good_value = 2.0;
      _bad_value = -1.0;
      _value = _default_value;
    }

  bool is_valid(double value) { return value > 0.8; }
};


}
#endif
