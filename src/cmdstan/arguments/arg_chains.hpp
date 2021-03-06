#ifndef CMDSTAN_ARGUMENTS_ARG_CHAINS_HPP
#define CMDSTAN_ARGUMENTS_ARG_CHAINS_HPP

#include <cmdstan/arguments/arg_num_chains.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

  class arg_chains: public categorical_argument {
  public:
    arg_chains() {
      _name = "chains";
      _description = "Number of Chains";

      _subarguments.push_back(new arg_num_chains());

    }
  };

}
#endif
