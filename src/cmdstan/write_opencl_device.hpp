#ifndef CMDSTAN_WRITE_OPENCL_DEVICE_HPP
#define CMDSTAN_WRITE_OPENCL_DEVICE_HPP

#include <stan/callbacks/writer.hpp>
#include <stan/callbacks/structured_writer.hpp>
#ifdef STAN_OPENCL
#include <stan/math/opencl/opencl_context.hpp>
#endif
#include <string>

namespace cmdstan {

void write_opencl_device(stan::callbacks::writer &writer) {
#ifdef STAN_OPENCL
  if ((stan::math::opencl_context.platform().size() > 0)
      && (stan::math::opencl_context.device().size() > 0)) {
    std::stringstream msg_opencl_platform;
    msg_opencl_platform
        << "opencl_platform_name = "
        << stan::math::opencl_context.platform()[0].getInfo<CL_PLATFORM_NAME>();
    writer(msg_opencl_platform.str());
    std::stringstream msg_opencl_device;
    msg_opencl_device
        << "opencl_device_name = "
        << stan::math::opencl_context.device()[0].getInfo<CL_DEVICE_NAME>();
    writer(msg_opencl_device.str());
  }
#endif
}

void write_opencl_device(stan::callbacks::structured_writer &writer) {
#ifdef STAN_OPENCL
  if ((stan::math::opencl_context.platform().size() > 0)
      && (stan::math::opencl_context.device().size() > 0)) {
    std::stringstream msg_opencl_platform;
    msg_opencl_platform
        << stan::math::opencl_context.platform()[0].getInfo<CL_PLATFORM_NAME>();
    writer.write("opencl_platform_name", msg_opencl_platform.str());
    std::stringstream msg_opencl_device;
    msg_opencl_device
        << stan::math::opencl_context.device()[0].getInfo<CL_DEVICE_NAME>();
    writer.write("opencl_device_name", msg_opencl_device.str());
  }
#endif
}

}  // namespace cmdstan
#endif
