#ifndef CMDSTAN_ARGUMENTS_ARG_DATA_FILE_HPP
#define CMDSTAN_ARGUMENTS_ARG_DATA_FILE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_data_file : public string_argument {
 public:
  arg_data_file() : string_argument() {
    _name = "file";
    _description = "Input data file";
    _validity = "Path to existing file";
    _default = "\"\"";
    _default_value = "";
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
