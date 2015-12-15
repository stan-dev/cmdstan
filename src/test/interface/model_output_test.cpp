#include <gtest/gtest.h>
#include <stan/services/error_codes.hpp>
#include <test/utility.hpp>

using cmdstan::test::convert_model_path;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;
using cmdstan::test::count_matches;

TEST(interface, check_model_output) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("printer");

  std::string command 
    = convert_model_path(model_path)
    + " sample num_warmup=0 num_samples=0"
    + " output file=" + convert_model_path(model_path) + ".csv";
  
  run_command_output out = run_command(command);
  EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code);
  EXPECT_FALSE(out.hasError);

  EXPECT_TRUE(count_matches("\nz=", out.body) > 2)
    << "counted " << count_matches("\nz=", out.body) << " instances of \"z=\" in"
    << std::endl
    << out.body;
  EXPECT_TRUE(count_matches("\ny=", out.body) > 2)
    << "counted " << count_matches("\ny=", out.body) << " instances of \"z=\" in"
    << std::endl
    << out.body;
}
