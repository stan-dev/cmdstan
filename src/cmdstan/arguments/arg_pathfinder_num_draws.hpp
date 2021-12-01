#ifndef CMDSTAN_ARGUMENTS_ARG_PATHFINDER_NUM_DRAWS_HPP
#define CMDSTAN_ARGUMENTS_ARG_PATHFINDER_NUM_DRAWS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_num_draws : public int_argument {
 public:
  arg_num_draws(const char* name = "num_draws",
   const char* desc = "Total number of draws", int def = 2000, const char* def_str = "2000") : int_argument() {
    _name = name;
    _description = desc;
    _validity = "0 < num_draws";
    _default = def_str;
    _default_value = def;
    _constrained = true;
    _good_value = 2.0;
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(int value) { return value > 0; }
};

}  // namespace cmdstan
#endif
