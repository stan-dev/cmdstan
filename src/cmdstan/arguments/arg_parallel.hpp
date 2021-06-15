#ifndef CMDSTAN_ARGUMENTS_ARG_PARALLEL_HPP
#define CMDSTAN_ARGUMENTS_ARG_PARALLEL_HPP

#include <cmdstan/arguments/arg_num_chains.hpp>
#include <cmdstan/arguments/arg_num_threads.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_parallel : public categorical_argument {
 public:
  arg_parallel() {
    _name = "parallel";
    _description = "Options for parallelism";
    _subarguments.push_back(new arg_num_chains());
    _subarguments.push_back(new arg_num_threads());
  }
};

}  // namespace cmdstan
#endif
