#ifndef CMDSTAN_ARGUMENTS_ARG_ADAPT_DELTA_HPP
#define CMDSTAN_ARGUMENTS_ARG_ADAPT_DELTA_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {


  class arg_adapt_delta: public real_argument {
  public:
    arg_adapt_delta(): real_argument() {
      _name = "delta";
      _description = "Adaptation target acceptance statistic";
      _validity = "0 < delta < 1";
      _default = "0.8";
      _default_value = 0.8;
      _constrained = true;
      _good_value = 0.5;
      _bad_value = -1.0;
      _value = _default_value;
    }

    bool is_valid(double value) { return 0 < value && value < 1; }
  };

}
#endif
