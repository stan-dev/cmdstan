#include <test/utility.hpp>
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <stdexcept>
#include <stan/io/stan_csv_reader.hpp>

using cmdstan::test::convert_model_path;
using cmdstan::test::multiple_command_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    multi_normal_model = {"src", "test", "test-models", "multi_normal_model"};
    multi_normal_optimized_params
        = {"src", "test", "test-models", "multi_normal_optimized_params.csv"};
    default_file_path = {"src", "test", "test-models", "output.csv"};
    dev_null_path = {"/dev", "null"};
  }
  std::vector<std::string> multi_normal_model;
  std::vector<std::string> multi_normal_optimized_params;
  std::vector<std::string> default_file_path;
  std::vector<std::string> dev_null_path;
};

TEST_F(CmdStan, laplace_good) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode="
     << convert_model_path(multi_normal_optimized_params);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, laplace_no_mode_arg) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, laplace_missing_mode) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode="
     << convert_model_path(multi_normal_optimized_params) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}
