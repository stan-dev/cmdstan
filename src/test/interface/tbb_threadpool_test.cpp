#include <cmdstan/command.hpp>
#include <gtest/gtest.h>
#include <test/test-models/proper.hpp>

#include <tbb/task_scheduler_init.h>
#include <stan/math/prim/core/init_threadpool_tbb.hpp>

TEST(StanUiCommand, threadpool_init) {
  tbb::task_scheduler_init& scheduler_init = stan::math::init_threadpool_tbb();
  EXPECT_TRUE(scheduler_init.is_active());
  scheduler_init.terminate();
  EXPECT_FALSE(scheduler_init.is_active());
}
