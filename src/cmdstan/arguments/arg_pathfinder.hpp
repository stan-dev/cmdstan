#ifndef CMDSTAN_ARGUMENTS_ARG_PATHFINDER_HPP
#define CMDSTAN_ARGUMENTS_ARG_PATHFINDER_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_single_bool.hpp>
#include <cmdstan/arguments/arg_single_int_pos.hpp>
#include <cmdstan/arguments/arg_lbfgs.hpp>

namespace cmdstan {

class arg_pathfinder : public arg_lbfgs {
 public:
  arg_pathfinder() {
    _name = "pathfinder";
    _description = "Pathfinder algorithm";

    _subarguments.push_back(new arg_single_int_pos(
        "num_psis_draws", "Number of draws from PSIS sample", 1000));
    _subarguments.push_back(
        new arg_single_int_pos("num_paths", "Number of single pathfinders", 4));
    _subarguments.push_back(new arg_single_bool(
        "save_single_paths", "Output single-path pathfinder draws as CSV",
        false));
    _subarguments.push_back(new arg_single_int_pos(
        "max_lbfgs_iters", "Maximum number of LBFGS iterations", 1000));
    _subarguments.push_back(new arg_single_int_pos(
        "num_draws", "Number of approximate posterior draws", 1000));
    _subarguments.push_back(new arg_single_int_pos(
        "num_elbo_draws", "Number of Monte Carlo draws to evaluate ELBO", 25));
  }
};

}  // namespace cmdstan

#endif
