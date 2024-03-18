#ifndef CMDSTAN_ARGUMENTS_ARG_OUTPUT_HPP
#define CMDSTAN_ARGUMENTS_ARG_OUTPUT_HPP

#include <cmdstan/arguments/arg_diagnostic_file.hpp>
#include <cmdstan/arguments/arg_output_file.hpp>
#include <cmdstan/arguments/arg_output_sig_figs.hpp>
#include <cmdstan/arguments/arg_profile_file.hpp>
#include <cmdstan/arguments/arg_refresh.hpp>
#include <cmdstan/arguments/arg_single_bool.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_output : public categorical_argument {
 public:
  arg_output() {
    _name = "output";
    _description = "File output options";

    _subarguments.push_back(new arg_output_file());
    _subarguments.push_back(new arg_diagnostic_file());
    _subarguments.push_back(new arg_refresh());
    _subarguments.push_back(new arg_output_sig_figs());
    _subarguments.push_back(new arg_profile_file());
    _subarguments.push_back(new arg_single_bool(
        "save_cmdstan_config",
        "Save the CmdStan configuration (parsed arguments + default values) as "
        "JSON alongside the output files",
        false));
  }
};

}  // namespace cmdstan
#endif
