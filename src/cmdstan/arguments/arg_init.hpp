#ifndef CMDSTAN_ARGUMENTS_ARG_INIT_HPP
#define CMDSTAN_ARGUMENTS_ARG_INIT_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <string>

namespace cmdstan {

class arg_init : public string_argument {
 public:
  arg_init() : string_argument() {
    _name = "init";
    _description = std::string("Initialization method: ")
                   + std::string("\"x\" initializes randomly between [-x, x], ")
                   + std::string("\"0\" initializes to 0, ")
                   + std::string("anything else identifies a file of values");
    _default = "\"2\"";
    _default_value = "2";
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
