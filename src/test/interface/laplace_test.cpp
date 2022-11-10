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
    multi_normal_mode_csv
        = {"src", "test", "test-models", "multi_normal_mode.csv"};
    multi_normal_mode_json
        = {"src", "test", "test-models", "multi_normal_mode.json"};
    default_file_path = {"src", "test", "test-models", "output.csv"};
    dev_null_path = {"/dev", "null"};
    wrong_csv = {"src", "test", "test-models", "bern_fitted_params.csv"};
    log_jacobian_model = {"src", "test", "test-models", "log_jacobian_sigma"};
    log_jacobian_mode_json
        = {"src", "test", "test-models", "log_jacobian_mode.json"};
  }
  std::vector<std::string> default_file_path;
  std::vector<std::string> dev_null_path;
  std::vector<std::string> log_jacobian_model;
  std::vector<std::string> log_jacobian_mode_json;
  std::vector<std::string> multi_normal_model;
  std::vector<std::string> multi_normal_mode_csv;
  std::vector<std::string> multi_normal_mode_json;
  std::vector<std::string> wrong_csv;
};

TEST_F(CmdStan, laplace_good) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode="
     << convert_model_path(multi_normal_mode_csv);
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);

  ss.str(std::string());
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode="
     << convert_model_path(multi_normal_mode_json);
  out = run_command(ss.str());
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
  std::cout << out.output << std::endl;
}

TEST_F(CmdStan, laplace_wrong_mode_file) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode="
     << convert_model_path(wrong_csv);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
  std::cout << out.output << std::endl;
}

TEST_F(CmdStan, laplace_missing_mode) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode=" << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
  std::cout << out.output << std::endl;
}

TEST_F(CmdStan, laplace_bad_draws_arg) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode="
     << convert_model_path(multi_normal_mode_csv)
     << " draws=0 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
  std::cout << out.output << std::endl;
}

TEST_F(CmdStan, laplace_jacobian) {
  std::stringstream ss;
  ss << convert_model_path(log_jacobian_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode="
     << convert_model_path(log_jacobian_mode_json)
     << "  2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
  std::cout << out.output << std::endl;
