#ifndef CMDSTAN_ARGUMENTS_ARG_OPENCL_PLATFORM_HPP
#define CMDSTAN_ARGUMENTS_ARG_OPENCL_PLATFORM_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_opencl_platform : public int_argument {
 public:
  arg_opencl_platform() : int_argument() {
    _name = "platform";
    _description = "ID of the OpenCL platform to use";
    _validity = "platform >= 0 or -1 to use the compile-time platform ID";
    _default = "-1";
    _default_value = -1;
    _value = _default_value;
  }

  bool is_valid(int value) { return value >= 0 || value == _default_value; }
};

}  // namespace cmdstan
#endif
