#ifndef CMDSTAN_ARGUMENTS_ARG_DATA_HPP
#define CMDSTAN_ARGUMENTS_ARG_DATA_HPP

#include <cmdstan/arguments/categorical_argument.hpp>

#include <cmdstan/arguments/arg_data_file.hpp>

namespace cmdstan {

  class arg_data: public categorical_argument {
  public:
    arg_data(): categorical_argument() {
      _name = "data";
      _description = "Input data options";

      _subarguments.push_back(new arg_data_file());
    }
  };

}
#endif
