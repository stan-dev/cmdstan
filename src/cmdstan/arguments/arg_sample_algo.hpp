#ifndef CMDSTAN_ARGUMENTS_ARG_SAMPLE_ALGO_HPP
#define CMDSTAN_ARGUMENTS_ARG_SAMPLE_ALGO_HPP

#include <cmdstan/arguments/list_argument.hpp>
#include <cmdstan/arguments/arg_hmc.hpp>
#include <cmdstan/arguments/arg_fixed_param.hpp>

namespace cmdstan {

  class arg_sample_algo: public list_argument {
  public:
    arg_sample_algo() {
      _name = "algorithm";
      _description = "Sampling algorithm";

      _values.push_back(new arg_hmc());
      _values.push_back(new arg_fixed_param());

      _default_cursor = 0;
      _cursor = _default_cursor;
    }
  };

}
#endif
