#ifndef CMDSTAN_ARGUMENTS_ARG_SINGLE_U_INT_HPP
#define CMDSTAN_ARGUMENTS_ARG_SINGLE_U_INT_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <string>

/** Generic unsigned int value argument */

namespace cmdstan {

class arg_single_u_int : public u_int_argument {
 public:
  arg_single_u_int(const char* name, const char* desc, unsigned int def)
      : u_int_argument() {
    _name = name;
    _description = desc;
    _validity = std::string("0 < ").append(name);
    _default = std::to_string(def);
    _default_value = def;
    _value = _default_value;
  }

  bool is_valid(unsigned int value) { return true; }
};

}  // namespace cmdstan
#endif
