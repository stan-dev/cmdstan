#ifndef CMDSTAN_ARGUMENTS_ARG_METRIC_FILE_HPP
#define CMDSTAN_ARGUMENTS_ARG_METRIC_FILE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

  class arg_metric_file: public string_argument {
  public:
    arg_metric_file(): string_argument() {
      _name = "metric_file";
      _description = "Input file with precomputed Euclidean metric";
      _validity = "Path to existing file";
      _default = "\"\"";
      _default_value = "";
      _constrained = false;
      _good_value = "";
      _value = _default_value;
    }
  };

}

#endif
