#ifndef CMDSTAN_ARGUMENTS_ARG_PATHFINDER_ALGO_HPP
#define CMDSTAN_ARGUMENTS_ARG_PATHFINDER_ALGO_HPP

#include <cmdstan/arguments/arg_pathfinder_single.hpp>
#include <cmdstan/arguments/arg_pathfinder_multi.hpp>
#include <cmdstan/arguments/arg_newton.hpp>
#include <cmdstan/arguments/list_argument.hpp>

namespace cmdstan {

class arg_pathfinder_algo : public list_argument {
 public:
  arg_pathfinder_algo() {
    _name = "algorithm";
    _description = "Pathfinder algorithm";

    _values.push_back(new arg_pathfinder_multi());
    _values.push_back(new arg_pathfinder_single());

    _default_cursor = 1;
    _cursor = _default_cursor;
  }
};

}  // namespace cmdstan
#endif
