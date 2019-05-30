#ifndef CMDSTAN_ARGUMENTS_ARG_SEED_HPP
#define CMDSTAN_ARGUMENTS_ARG_SEED_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {


  class arg_seed: public int_argument {
  public:
    arg_seed(): int_argument() {
      _name = "seed";
      _description = "Random number generator seed";
      _validity = "integer > 0 or -1 to generate seed from system time";
      _default = "-1";
      _default_value = -1;
      _constrained = true;
      _good_value = 18383;
      _bad_value = -2;
      _value = _default_value;
    }

    bool is_valid(int value) {
      return value > 0 || value == _default_value;
    }
  };

}
#endif
