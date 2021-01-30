#ifndef CMDSTAN_VARIATIONAL_WINDOW_SIZE_HPP
#define CMDSTAN_VARIATIONAL_WINDOW_SIZE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_variational_window_size : public real_argument {
 public:
  arg_variational_window_size() : real_argument() {
    _name = "window_size";
    _description = "Proportion of eval_window samples"
                    "to calculate Rhat in pre-iter";
    _validity = "0.0 < window_size <= 1.0";
    _default = "0.5";
    _default_value = 0.5;
    _constrained = true;
    _good_value = 0.5;
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(int value) { return value > 0.0 && value <= 1.0; }
};

}  // namespace cmdstan
#endif
