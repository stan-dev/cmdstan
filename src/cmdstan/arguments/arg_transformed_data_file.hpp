#ifndef CMDSTAN_ARGUMENTS_ARG_TRANSFORMED_DATA_FILE_HPP
#define CMDSTAN_ARGUMENTS_ARG_TRANSFORMED_DATA_FILE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_transformed_data_file : public string_argument {
 public:
  arg_transformed_data_file() : string_argument() {
    _name = "transformed_data";
    _description
        = "Auxiliary output file for the contents of the transformed data "
          "block, written as JSON.";
    _validity = "File should not already exist";
    _default = "\"\"";
    _default_value = "";
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
