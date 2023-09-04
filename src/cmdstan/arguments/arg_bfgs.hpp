#ifndef CMDSTAN_ARGUMENTS_ARG_BFGS_HPP
#define CMDSTAN_ARGUMENTS_ARG_BFGS_HPP

#include <cmdstan/arguments/arg_single_real_pos.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_bfgs : public categorical_argument {
 public:
  arg_bfgs() {
    _name = "bfgs";
    _description = "BFGS with linesearch";

    _subarguments.push_back(new arg_single_real_pos(
        "init_alpha", "Line search step size for first iteration", 0.001));
    _subarguments.push_back(
        new arg_single_real_pos("tol_obj",
                                "Convergence tolerance on absolute changes "
                                "in objective function value",
                                1e-12));
    _subarguments.push_back(
        new arg_single_real_pos("tol_rel_obj",
                                "Convergence tolerance on relative changes "
                                "in objective function value",
                                1e+4));
    _subarguments.push_back(new arg_single_real_pos(
        "tol_grad", "Convergence tolerance on the norm of the gradient", 1e-8));
    _subarguments.push_back(
        new arg_single_real_pos("tol_rel_grad",
                                "Convergence tolerance on the relative norm "
                                "of the gradient",
                                1e+7));
    _subarguments.push_back(
        new arg_single_real_pos("tol_param",
                                "Convergence tolerance on changes "
                                "in parameter value",
                                1e-8));
  }
};

}  // namespace cmdstan
#endif
