#ifndef CMDSTAN_ARGUMENTS_ARG_OPTIMIZE_HPP
#define CMDSTAN_ARGUMENTS_ARG_OPTIMIZE_HPP

#include <cmdstan/arguments/arg_optimize_algo.hpp>
#include <cmdstan/arguments/arg_single_bool.hpp>
#include <cmdstan/arguments/arg_single_int_pos.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_optimize : public categorical_argument {
 public:
  arg_optimize() {
    _name = "optimize";
    _description = "Point estimation";

    _subarguments.push_back(new arg_optimize_algo());
    _subarguments.push_back(
        new arg_single_bool("jacobian",
                            "When true, include change-of-variables adjustment"
                            " for constraining parameter transforms",
                            false));
    _subarguments.push_back(
        new arg_single_int_pos("iter", "Total number of iterations", 2000));
    _subarguments.push_back(new arg_single_bool(
        "save_iterations", "Stream optimization progress to output?", false));
  }
};

}  // namespace cmdstan
#endif
