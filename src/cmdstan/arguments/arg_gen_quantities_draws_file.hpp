#ifndef CMDSTAN_ARGUMENTS_ARG_GEN_QUANTITIES_DRAWS_FILE_HPP
#define CMDSTAN_ARGUMENTS_ARG_GEN_QUANTITIES_DRAWS_FILE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

  class arg_gen_quantities_draws_file: public string_argument {
  public:
    arg_gen_quantities_draws_file(): string_argument() {
      _name = "draws_file";
      _description = "Input file of draws from fitted model";
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
