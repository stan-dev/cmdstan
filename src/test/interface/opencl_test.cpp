#ifdef STAN_OPENCL

#include <stan/math.hpp>
#include <gtest/gtest.h>

TEST(StanUiCommand, opencl_ready) {
  // Check if  the OpenCL context has a device ready
  EXPECT_TRUE(
      stan::math::opencl_context.device()[0].getInfo<CL_DEVICE_AVAILABLE>());
}

#endif
