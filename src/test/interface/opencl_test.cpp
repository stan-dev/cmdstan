#ifdef STAN_OPENCL

#include <gtest/gtest.h>
#include <stan/math/opencl/opencl.hpp>

TEST(StanUiCommand, opencl_ready) {
  // Check if  the OpenCL context has a device ready
  EXPECT_TRUE(stan::math::opencl_context.device()[0].getInfo<CL_DEVICE_AVAILABLE>());
}

#endif
