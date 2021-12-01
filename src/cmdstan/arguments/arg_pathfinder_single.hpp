#ifndef CMDSTAN_ARGUMENTS_ARG_PATHFINDER_SINGLE_HPP
#define CMDSTAN_ARGUMENTS_ARG_PATHFINDER_SINGLE_HPP

#include <cmdstan/arguments/arg_lbfgs.hpp>

namespace cmdstan {

class arg_pathfinder_single : public categorical_argument {
 public:
  arg_pathfinder_single() {
    _name = "single";
    _description = "pathfinder with lbfgs";
    _subarguments.push_back(new arg_save_iterations());
  }
};

}  // namespace cmdstan
#endif
