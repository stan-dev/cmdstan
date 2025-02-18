#ifndef CMDSTAN_ARGUMENTS_ARG_DIAGNOSTIC_FILE_HPP
#define CMDSTAN_ARGUMENTS_ARG_DIAGNOSTIC_FILE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_diagnostic_file : public string_argument {
 public:
  arg_diagnostic_file() : string_argument() {
    _name = "diagnostic_file";
    _description
        = "Auxiliary output file for diagnostic information. If multiple "
          "chains are run, this can either be a single path, in which case its "
          "name will have _ID appended, or a comma-separated list of names.";
    _validity = "File(s) should not already exist";
    _default = "\"\"";
    _default_value = "";
    _value = _default_value;
  }
};

}  // namespace cmdstan
#endif
