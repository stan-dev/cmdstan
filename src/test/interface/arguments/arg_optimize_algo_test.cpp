#include <cmdstan/arguments/arg_optimize_algo.hpp>
#include <gtest/gtest.h>

TEST(CmdStanArguments, arg_optimize_algo) {
  cmdstan::arg_optimize_algo arg;

  EXPECT_EQ("algorithm", arg.name());
  EXPECT_EQ("Optimization algorithm", arg.description());

  ASSERT_EQ(3U, arg.values().size());
  EXPECT_EQ("bfgs", arg.values()[0]->name());
  EXPECT_EQ("lbfgs", arg.values()[1]->name());
  EXPECT_EQ("newton", arg.values()[2]->name());
}
