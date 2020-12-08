#ifndef CMDSTAN_ARGUMENTS_ARG_OPENCL_HPP
#define CMDSTAN_ARGUMENTS_ARG_OPENCL_HPP

#include <cmdstan/arguments/arg_opencl_device.hpp>
#include <cmdstan/arguments/arg_opencl_platform.hpp>
#include <cmdstan/arguments/categorical_argument.hpp>

namespace cmdstan {

class arg_opencl : public categorical_argument {
 public:
  arg_opencl() {
    _name = "opencl";
    _description = "OpenCL options";

    _subarguments.push_back(new arg_opencl_device());
    _subarguments.push_back(new arg_opencl_platform());
  }
};

}  // namespace cmdstan
#endif
