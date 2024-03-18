#ifndef CMDSTAN_ARGUMENTS_ARG_ADAPT_HPP
#define CMDSTAN_ARGUMENTS_ARG_ADAPT_HPP

#include <cmdstan/arguments/categorical_argument.hpp>
#include <cmdstan/arguments/arg_single_bool.hpp>
#include <cmdstan/arguments/arg_single_int_pos.hpp>
#include <cmdstan/arguments/arg_single_u_int.hpp>
#include <cmdstan/arguments/arg_single_real_bounded.hpp>
#include <cmdstan/arguments/arg_single_real_pos.hpp>
#include <cmdstan/arguments/arg_single_string.hpp>
#include <cmath>
namespace cmdstan {
class arg_adapt : public categorical_argument {
 public:
  arg_adapt() {
    _name = "adapt";
    _description = "Warmup Adaptation";

    _subarguments.push_back(
        new arg_single_bool("engaged", "Adaptation engaged?", true));
    _subarguments.push_back(new arg_single_real_pos(
        "gamma", "Adaptation regularization scale", 0.05));
    _subarguments.push_back(new arg_single_real_bounded(
        "delta", "Adaptation target acceptance statistic", 0.8, 0.1,
        std::nextafter(1.0, 0.0)));
    _subarguments.push_back(new arg_single_real_pos(
        "kappa", "Adaptation relaxation exponent", 0.75));
    _subarguments.push_back(
        new arg_single_real_pos("t0", "Adaptation iteration offset", 10));
    _subarguments.push_back(new arg_single_u_int(
        "init_buffer", "Width of initial fast adaptation interval", 75));
    _subarguments.push_back(new arg_single_u_int(
        "term_buffer", "Width of final fast adaptation interval", 50));
    _subarguments.push_back(new arg_single_u_int(
        "window", "Initial width of slow adaptation interval", 25));
    _subarguments.push_back(
        new arg_single_bool("save_metric", "Save metric as JSON?", false));
  }
};

}  // namespace cmdstan
#endif
