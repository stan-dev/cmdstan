#ifndef CMDSTAN_ARGUMENTS_ARG_STATIC_HPP
#define CMDSTAN_ARGUMENTS_ARG_STATIC_HPP

#include <cmdstan/arguments/arg_single_real_pos.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_static : public categorical_argument {
 public:
  arg_static() {
    _name = "static";
    _description = "Static integration time";
    _subarguments.push_back(new arg_single_real_pos(
        "int_time",
        "Total integration time for Hamiltonian evolution, default is 2 * pi",
        6.28318530717959));
  }
};

}  // namespace cmdstan
#endif
