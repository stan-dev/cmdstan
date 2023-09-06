#ifndef CMDSTAN_ARGUMENTS_VARIATIONAL_HPP
#define CMDSTAN_ARGUMENTS_VARIATIONAL_HPP

#include <cmdstan/arguments/arg_single_int_pos.hpp>
#include <cmdstan/arguments/arg_single_real_pos.hpp>
#include <cmdstan/arguments/arg_variational_adapt.hpp>
#include <cmdstan/arguments/arg_variational_algo.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>
#include <stan/services/experimental/advi/defaults.hpp>
#include <string>

namespace cmdstan {

using stan::services::experimental::advi::adapt_iterations;
using stan::services::experimental::advi::elbo_samples;
using stan::services::experimental::advi::eta;
using stan::services::experimental::advi::eval_elbo;
using stan::services::experimental::advi::gradient_samples;
using stan::services::experimental::advi::max_iterations;
using stan::services::experimental::advi::output_draws;
using stan::services::experimental::advi::tol_rel_obj;

class arg_variational : public categorical_argument {
 public:
  arg_variational() {
    _name = "variational";
    _description = "Variational inference";

    _subarguments.push_back(new arg_variational_algo());
    _subarguments.push_back(
        new arg_single_int_pos("iter", max_iterations::description().c_str(),
                               max_iterations::default_value()));
    _subarguments.push_back(new arg_single_int_pos(
        "grad_samples", gradient_samples::description().c_str(),
        gradient_samples::default_value()));
    _subarguments.push_back(new arg_single_int_pos(
        "elbo_samples", elbo_samples::description().c_str(),
        elbo_samples::default_value()));
    _subarguments.push_back(new arg_single_real_pos(
        "eta", eta::description().c_str(), eta::default_value()));
    _subarguments.push_back(new arg_variational_adapt());
    _subarguments.push_back(new arg_single_real_pos(
        "tol_rel_obj", tol_rel_obj::description().c_str(),
        tol_rel_obj::default_value()));
    _subarguments.push_back(
        new arg_single_int_pos("eval_elbo", eval_elbo::description().c_str(),
                               eval_elbo::default_value()));
    _subarguments.push_back(new arg_single_int_pos(
        "output_samples", output_draws::description().c_str(),
        output_draws::default_value()));
  }
};

}  // namespace cmdstan
#endif
