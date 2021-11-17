#ifndef CMDSTAN_ARGUMENTS_ARG_PATHFINDER_NUM_DRAWS_HPP
#define CMDSTAN_ARGUMENTS_ARG_PATHFINDER_NUM_DRAWS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_num_draws : public int_argument {
 public:
  arg_num_draws() : int_argument() {
    _name = "num_draws";
    _description = "Total number of num_drawsations";
    _validity = "0 < num_draws";
    _default = "2000";
    _default_value = 2000;
    _constrained = true;
    _good_value = 2.0;
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(int value) { return value > 0; }
};

}  // namespace cmdstan
#endif
