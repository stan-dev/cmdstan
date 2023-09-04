#ifndef CMDSTAN_ARGUMENTS_ARG_SAMPLE_HPP
#define CMDSTAN_ARGUMENTS_ARG_SAMPLE_HPP

#include <cmdstan/arguments/arg_adapt.hpp>
#include <cmdstan/arguments/arg_sample_algo.hpp>
#include <cmdstan/arguments/arg_single_bool.hpp>
#include <cmdstan/arguments/arg_single_int_nonneg.hpp>
#include <cmdstan/arguments/arg_single_int_pos.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_sample : public categorical_argument {
 public:
  arg_sample() {
    _name = "sample";
    _description = "Bayesian inference with Markov Chain Monte Carlo";
    _subarguments.push_back(new arg_single_int_nonneg(
        "num_samples", "Number of sampling iterations", 1000));
    _subarguments.push_back(new arg_single_int_nonneg(
        "num_warmup", "Number of warmup iterations", 1000));
    _subarguments.push_back(new arg_single_bool(
        "save_warmup", "Stream warmup samples to output?", false));
    _subarguments.push_back(
        new arg_single_int_pos("thin", "Period between saved samples", 1));
    _subarguments.push_back(new arg_adapt());
    _subarguments.push_back(new arg_sample_algo());
    _subarguments.push_back(
        new arg_single_int_pos("num_chains", "Number of chains", 1));
  }
};

}  // namespace cmdstan
#endif
