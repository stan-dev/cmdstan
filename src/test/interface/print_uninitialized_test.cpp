#include <gtest/gtest.h>
#include <stan/services/error_codes.hpp>
#include <test/utility.hpp>

using cmdstan::test::convert_model_path;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

TEST(interface,print_uninitialized) {
  // This was stan-dev/stan issue #91
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("print_uninitialized");

  std::string command 
    = convert_model_path(model_path)
    + " sample num_warmup=0 num_samples=0"
    + " output file=" + convert_model_path(model_path) + ".csv";
  
  run_command_output out = run_command(command);
  EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code);
  EXPECT_FALSE(out.hasError);
}
