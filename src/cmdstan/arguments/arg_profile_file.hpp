#ifndef CMDSTAN_ARGUMENTS_ARG_PROFILE_FILE_HPP
#define CMDSTAN_ARGUMENTS_ARG_PROFILE_FILE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_profile_file : public string_argument {
 public:
  arg_profile_file() : string_argument() {
    _name = "profile_file";
    _description = "File to store profiling information";
    _validity = "Valid path and write access to the folder";
    _default = "\"\"";
    _default_value = "profile.csv";
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
