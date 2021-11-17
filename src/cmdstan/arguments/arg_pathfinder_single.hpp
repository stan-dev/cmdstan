#ifndef CMDSTAN_ARGUMENTS_ARG_PATHFINDER_SINGLE_HPP
#define CMDSTAN_ARGUMENTS_ARG_PATHFINDER_SINGLE_HPP

#include <cmdstan/arguments/arg_pathfinder_single.hpp>

namespace cmdstan {

class arg_pathfinder_single : public arg_lbfgs {
 public:
  arg_pathfinder_single() {
    _name = "single";
    _description = "pathfinder with lbfgs";
  }
};

}  // namespace cmdstan
#endif
