#ifndef CMDSTAN_ARGUMENTS_ARG_OUTPUT_FILE_HPP
#define CMDSTAN_ARGUMENTS_ARG_OUTPUT_FILE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_output_file : public string_argument {
 public:
  arg_output_file() : string_argument() {
    _name = "file";
    _description
        = "Output file.  If multiple chains are run, this can either be a "
          "single path, in which case its name will have _ID appended, or a "
          "comma-separated list of names.";
    _validity = "File(s) should not already exist";
    _default = "output.csv";
    _default_value = "output.csv";
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
