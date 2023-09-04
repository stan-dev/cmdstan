#ifndef CMDSTAN_ARGUMENTS_VARIATIONAL_HPP
#define CMDSTAN_ARGUMENTS_VARIATIONAL_HPP

#include <cmdstan/arguments/arg_single_int_pos.hpp>
#include <cmdstan/arguments/arg_single_real_pos.hpp>
#include <cmdstan/arguments/arg_single_string.hpp>
#include <cmdstan/arguments/arg_variational_adapt.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_variational : public categorical_argument {
 public:
  arg_variational() {
    _name = "variational";
    _description = "Variational inference";

    _subarguments.push_back(new arg_single_string(
        "algorithm", "Variational inference algorithm.", "meanfield"));
    _subarguments.push_back(new arg_single_int_pos(
        "iter", "Maximum number of ADVI iterations.", 1e+4));
    _subarguments.push_back(new arg_single_int_pos(
        "grad_samples",
        "Number of Monte Carlo draws for computing the gradient.",
        1));
    _subarguments.push_back(new arg_single_int_pos(
        "elbo_samples",
        "Number of Monte Carlo draws for estimate of ELBO.",
        100));
    _subarguments.push_back(new arg_single_real_pos(
        "eta", "Stepsize scaling parameter.", 1.0));

    _subarguments.push_back(new arg_variational_adapt());

    _subarguments.push_back(new arg_single_real_pos(
        "tol_rel_obj", "Relative tolerance parameter for convergence", 0.01));
    _subarguments.push_back(new arg_single_int_pos(
        "eval_elbo", "Number of iterations between ELBO evaluations", 100));
    _subarguments.push_back(new arg_single_int_pos(
        "output_samples",
        "Number of approximate posterior output draws to save.",
        1000));
  }
};

}  // namespace cmdstan
#endif
