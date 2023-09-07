#ifndef CMDSTAN_ARGUMENTS_ARG_LOG_PROB_UNCONSTRAINED_PARAMS_HPP
#define CMDSTAN_ARGUMENTS_ARG_LOG_PROB_UNCONSTRAINED_PARAMS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {
/**
 * Argument for providing a file of parameters on the unconstrained scale
 * for use with the 'log_prob' method. The file can be in CSV or JSON format
 * and should contain a variable 'params_r' with either a vector or list/array
 * of vectors of unconstrained parameter values. Like the 'init' argument, if
 * the file has a '.json' extension it is treated as a JSON file, otherwise it
 * is treated as an RDump file.
 */
class arg_log_prob_unconstrained_params : public string_argument {
 public:
  arg_log_prob_unconstrained_params() : string_argument() {
    _name = "unconstrained_params";
    _description
        = "Input file (JSON or R dump) of parameter values on unconstrained "
          "scale";
    _validity = "Path to existing file";
    _default = "\"\"";
    _default_value = "";
    _value = _default_value;
  }
};

}  // namespace cmdstan

#endif
