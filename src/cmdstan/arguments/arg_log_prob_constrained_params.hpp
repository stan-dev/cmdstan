#ifndef CMDSTAN_ARGUMENTS_ARG_LOG_PROB_CONSTRAINED_PARAMS_HPP
#define CMDSTAN_ARGUMENTS_ARG_LOG_PROB_CONSTRAINED_PARAMS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_log_prob_constrained_params : public string_argument {
 public:
  arg_log_prob_constrained_params() : string_argument() {
    _name = "constrained_params";
    _description = "Input file of parameter values on constrained scale";
    _validity = "Path to existing file";
    _default = "\"\"";
    _default_value = "";
    _constrained = false;
    _good_value = "";
    _value = _default_value;
  }
};

}  // namespace cmdstan

#endif
