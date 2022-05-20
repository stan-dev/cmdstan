#ifndef CMDSTAN_ARGUMENTS_ARG_PATHFINDER_NUM_EVAL_ATTEMPTS_HPP
#define CMDSTAN_ARGUMENTS_ARG_PATHFINDER_NUM_EVAL_ATTEMPTS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_num_eval_attempts : public int_argument {
 public:
  arg_num_eval_attempts(const char* name = "num_eval_attempts",
   const char* desc = "Total number of attempts at evaluating log_prob before failing", int def = 2000, const char* def_str = "2000") : int_argument() {
    _name = name;
    _description = desc;
    _validity = "0 < num_eval_attempts";
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
