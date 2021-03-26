#include <cmdstan/command.hpp>
#include <test/test-models/proper.hpp>
#include <stan/math/prim/core/init_threadpool_tbb.hpp>
#include <gtest/gtest.h>

TEST(StanUiCommand, threadpool_init) {
  auto& scheduler_init = stan::math::init_threadpool_tbb();
  EXPECT_TRUE(scheduler_init.is_active());
  scheduler_init.terminate();
  EXPECT_FALSE(scheduler_init.is_active());
}
