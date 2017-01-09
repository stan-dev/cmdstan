#ifndef CMDSTAN_ARGUMENTS_ARG_OUTPUT_HPP
#define CMDSTAN_ARGUMENTS_ARG_OUTPUT_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_output_file.hpp>
#include <cmdstan/arguments/arg_diagnostic_file.hpp>
#include <cmdstan/arguments/arg_refresh.hpp>

namespace cmdstan {

  class arg_output: public categorical_argument {
  public:
    arg_output() {
      _name = "output";
      _description = "File output options";

      _subarguments.push_back(new arg_output_file());
      _subarguments.push_back(new arg_diagnostic_file());
      _subarguments.push_back(new arg_refresh());
    }
  };

}
#endif
