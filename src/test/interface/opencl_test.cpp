#ifdef STAN_OPENCL

#include <cmdstan/command.hpp>
#include <gtest/gtest.h>
#include <CL/cl.hpp>
#include <stan/math/prim/arr.hpp>

TEST(StanUiCommand, opencl_ready) {
  // Check if  the OpenCL context has a device ready
  EXPECT_TRUE(stan::math::opencl_context.device()[0].getInfo<CL_DEVICE_AVAILABLE>());
}

#endif
