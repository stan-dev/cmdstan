#ifndef CMDSTAN_ARGUMENTS_ARG_SINGLE_STRING_HPP
#define CMDSTAN_ARGUMENTS_ARG_SINGLE_STRING_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <string>

/** Generic string value argument */

namespace cmdstan {

class arg_single_string : public string_argument {
 public:
  arg_single_string(const char* name, const char* desc, const char* def)
      : string_argument() {
    _name = name;
    _description = desc;
    _default = def;
    _default_value = def;
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
