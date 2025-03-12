#include <cmdstan/stansummary_helper.hpp>
#include <stan/io/stan_csv_reader.hpp>
#include <stan/services/error_codes.hpp>
#include <test/utility.hpp>
#include <gtest/gtest.h>

using cmdstan::test::convert_model_path;

TEST(interface, unwritable_file) {
  std::string model = convert_model_path(
      std::vector{"src", "test", "test-models", "test_model"});
  std::string output
      = convert_model_path(std::vector{"test", "output_unwritable.csv"});

  cmdstan::test::temporary_unwritable_file guard(output);

  std::string command
      = model + " sample num_warmup=1 num_samples=1 output file=" + output;

  cmdstan::test::run_command_output out = cmdstan::test::run_command(command);
  EXPECT_IN_STRING("Permission denied", out.output);
  EXPECT_TRUE(out.hasError);
}
