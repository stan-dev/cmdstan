#ifndef CMDSTAN_ARGUMENTS_ARG_SINGLE_BOOL_HPP
#define CMDSTAN_ARGUMENTS_ARG_SINGLE_BOOL_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <string>

/** Generic positive int value argument */

namespace cmdstan {

class arg_single_bool : public bool_argument {
 public:
  arg_single_bool(const char* name, const char* desc, bool def)
      : bool_argument() {
    _name = name;
    _description = desc;
    _validity = "[0, 1]";
    _default = std::to_string(def);
    _default_value = def;
    _constrained = false;
    _good_value = 1;
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
