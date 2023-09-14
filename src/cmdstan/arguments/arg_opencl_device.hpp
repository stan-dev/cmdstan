#ifndef CMDSTAN_ARGUMENTS_ARG_OPENCL_DEVICE_HPP
#define CMDSTAN_ARGUMENTS_ARG_OPENCL_DEVICE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_opencl_device : public int_argument {
 public:
  arg_opencl_device() : int_argument() {
    _name = "device";
    _description = "ID of the OpenCL device to use";
    _validity = "device >= 0 or -1 to use the compile-time device ID";
    _default = "-1";
    _default_value = -1;
    _value = _default_value;
  }

  bool is_valid(int value) { return value >= 0 || value == _default_value; }
};

}  // namespace cmdstan
#endif
