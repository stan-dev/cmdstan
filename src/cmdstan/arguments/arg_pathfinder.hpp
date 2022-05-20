#ifndef CMDSTAN_ARGUMENTS_ARG_PATHFINDER_HPP
#define CMDSTAN_ARGUMENTS_ARG_PATHFINDER_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_pathfinder_elbo_draws.hpp>
#include <cmdstan/arguments/arg_pathfinder_num_eval_attempts.hpp>
#include <cmdstan/arguments/arg_pathfinder_algo.hpp>
#include <cmdstan/arguments/arg_lbfgs.hpp>

namespace cmdstan {

class arg_pathfinder : public arg_lbfgs {
 public:
  arg_pathfinder() {
    _name = "pathfinder";
    _description = "Pathfinder Algorithm";

    _subarguments.push_back(new arg_pathfinder_algo());
    _subarguments.push_back(new arg_iter());
    _subarguments.push_back(new arg_num_elbo_draws());
    _subarguments.push_back(new arg_num_draws());
    _subarguments.push_back(new arg_num_eval_attempts());

  }
};

}  // namespace cmdstan

#endif
