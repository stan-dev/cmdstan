#ifndef CMDSTAN_ARGUMENTS_ARG_RANDOM_HPP
#define CMDSTAN_ARGUMENTS_ARG_RANDOM_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_seed.hpp>

namespace cmdstan {

  class arg_random: public categorical_argument {
  public:
    arg_random() {
      _name = "random";
      _description = "Random number configuration";

      _subarguments.push_back(new arg_seed());
    }
  };

}
#endif
