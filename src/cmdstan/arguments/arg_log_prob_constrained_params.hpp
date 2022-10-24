#ifndef CMDSTAN_ARGUMENTS_ARG_LOG_PROB_CONSTRAINED_PARAMS_HPP
#define CMDSTAN_ARGUMENTS_ARG_LOG_PROB_CONSTRAINED_PARAMS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {
/**
 * Argument for providing a file of parameters on the constrained scale
 * for use with the 'log_prob' method. The file can be in CSV or JSON format
 * and should contain a variable 'params_r' with either a vector or list/array
 * of vectors of constrained parameter values.
 */
class arg_log_prob_constrained_params : public string_argument {
 public:
  arg_log_prob_constrained_params() : string_argument() {
    _name = "constrained_params";
    _description
        = "Input file (JSON or R dump) of parameter values on constrained "
          "scale";
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
