#ifndef CMDSTAN_ARGUMENTS_ARG_GEN_QUANTITIES_HPP
#define CMDSTAN_ARGUMENTS_ARG_GEN_QUANTITIES_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_gen_quantities_draws_file.hpp>

namespace cmdstan {

  class arg_gen_quantities: public categorical_argument {
  public:
    arg_gen_quantities() {
      _name = "gen_quantities";
      _description = "Generate quantities of interest";

      _subarguments.push_back(new arg_gen_quantities_draws_file());
    }
  };

}
#endif
