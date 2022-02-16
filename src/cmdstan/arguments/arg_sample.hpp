#ifndef CMDSTAN_ARGUMENTS_ARG_SAMPLE_HPP
#define CMDSTAN_ARGUMENTS_ARG_SAMPLE_HPP

#include <cmdstan/arguments/arg_adapt.hpp>
#include <cmdstan/arguments/arg_num_chains.hpp>
#include <cmdstan/arguments/arg_num_samples.hpp>
#include <cmdstan/arguments/arg_num_warmup.hpp>
#include <cmdstan/arguments/arg_sample_algo.hpp>
#include <cmdstan/arguments/arg_save_warmup.hpp>
#include <cmdstan/arguments/arg_thin.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_parallel_tree : public bool_argument {
 public:
  arg_parallel_tree() : bool_argument() {
    _name = "parallel_tree";
    _description = "Evaluate both sides of tree eagerly?";
    _validity = "[0, 1]";
    _default = "0";
    _default_value = false;
    _constrained = false;
    _good_value = 1;
    _value = _default_value;
  }
};

class arg_sample : public categorical_argument {
 public:
  arg_sample() {
    _name = "sample";
    _description = "Bayesian inference with Markov Chain Monte Carlo";

    _subarguments.push_back(new arg_num_samples());
    _subarguments.push_back(new arg_num_warmup());
    _subarguments.push_back(new arg_save_warmup());
    _subarguments.push_back(new arg_thin());
    _subarguments.push_back(new arg_adapt());
    _subarguments.push_back(new arg_parallel_tree());
    _subarguments.push_back(new arg_sample_algo());
    _subarguments.push_back(new arg_num_chains());
  }
};

}  // namespace cmdstan
#endif
