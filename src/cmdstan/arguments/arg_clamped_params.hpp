#ifndef CMDSTAN_ARGUMENTS_ARG_CLAMPED_PARAMS_HPP
#define CMDSTAN_ARGUMENTS_ARG_CLAMPED_PARAMS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <string>

namespace cmdstan {

class arg_clamped_params : public string_argument {
 public:
  arg_clamped_params() : string_argument() {
    _name = "clamped_params";
    _description = std::string("File with clamped parameters");
    _default = "";
    _default_value = "";
    _constrained = false;
    _good_value = "clamped_params.json";
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
