#ifndef CMDSTAN_ARGUMENTS_ARG_PATHFINDER_MULTI_HPP
#define CMDSTAN_ARGUMENTS_ARG_PATHFINDER_MULTI_HPP

#include <cmdstan/arguments/arg_pathfinder_single.hpp>
#include <cmdstan/arguments/arg_pathfinder_num_draws.hpp>
#include <cmdstan/arguments/list_argument.hpp>

namespace cmdstan {

class arg_pathfinder_multi : public categorical_argument {
 public:
  arg_pathfinder_multi() {
    _name = "multi";
    _description = "pathfinder with pathfinder, u know";
    _subarguments.push_back(new arg_num_draws("psis_draws"));
    _subarguments.push_back(new arg_num_draws("num_paths", "# of single pathfinders", 4, "4"));
    _subarguments.push_back(new arg_save_iterations());

  }
};

}  // namespace cmdstan
#endif
