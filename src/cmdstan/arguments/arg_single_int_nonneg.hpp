#ifndef CMDSTAN_ARGUMENTS_ARG_SINGLE_INT_NONNEG_HPP
#define CMDSTAN_ARGUMENTS_ARG_SINGLE_INT_NONNEG_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <string>

/** Generic non-negative int value argument */

namespace cmdstan {

class arg_single_int_nonneg : public int_argument {
 public:
  arg_single_int_nonneg(const char* name, const char* desc, int def)
      : int_argument() {
    _name = name;
    _description = desc;
    _validity = std::string("0 < ").append(name);
    _default = std::to_string(def);
    _default_value = def;
    _value = _default_value;
  }

  bool is_valid(int value) { return value >= 0; }
};

}  // namespace cmdstan
#endif
