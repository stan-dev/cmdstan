#ifndef CMDSTAN_ARGUMENTS_ARG_SEED_HPP
#define CMDSTAN_ARGUMENTS_ARG_SEED_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {


  class arg_seed: public u_int_argument {
  public:
    arg_seed(): u_int_argument() {
      _name = "seed";
      _description = "Random number generator seed";
      _validity = "seed > 0, if negative seed is generated from time";
      _default = "-1";
      _default_value = -1;
      _constrained = false;
      _good_value = 18383;
      _value = _default_value;
    }
  };

}
#endif
