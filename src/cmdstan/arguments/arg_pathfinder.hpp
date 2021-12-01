#ifndef CMDSTAN_ARGUMENTS_ARG_PATHFINDER_HPP
#define CMDSTAN_ARGUMENTS_ARG_PATHFINDER_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_pathfinder_elbo_draws.hpp>
#include <cmdstan/arguments/arg_pathfinder_algo.hpp>

namespace cmdstan {

class arg_pathfinder : public categorical_argument {
 public:
  arg_pathfinder() {
    _name = "pathfinder";
    _description = "Pathfinder Algorithm";

    _subarguments.push_back(new arg_pathfinder_algo());
    _subarguments.push_back(new arg_iter());
    _subarguments.push_back(new arg_num_elbo_draws());
    _subarguments.push_back(new arg_num_draws());

  }
};

}  // namespace cmdstan

#endif
