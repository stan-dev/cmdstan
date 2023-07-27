#ifndef CMDSTAN_ARGUMENTS_VARIATIONAL_EVAL_ELBO_HPP
#define CMDSTAN_ARGUMENTS_VARIATIONAL_EVAL_ELBO_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <string>

namespace cmdstan {

class arg_variational_eval_elbo : public int_argument {
 public:
  arg_variational_eval_elbo(const char *name, const char *desc, double def)
      : int_argument() {
    _name = name;
    _description = desc;
    _validity = "0 < eval_elbo";
    _default = std::to_string(def);
    _default_value = def;
    _constrained = true;
    _good_value = 100.0;
    _bad_value = -1.0;
    _value = _default_value;
  }
  bool is_valid(int value) { return value > 0; }
};
}  // namespace cmdstan
#endif
