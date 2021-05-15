#include <test/utility.hpp>
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <stdexcept>

using cmdstan::test::convert_model_path;
using cmdstan::test::multiple_command_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    bern_gq_model = {"src", "test", "test-models", "bern_gq_model"};
    bern_extra_model = {"src", "test", "test-models", "bern_extra_model"};
    bern_data = {"src", "test", "test-models", "bern.data.json"};
    bern_fitted_params
        = {"src", "test", "test-models", "bern_fitted_params.csv"};
    default_file_path = {"src", "test", "test-models", "output.csv"};
    dev_null_path = {"/dev", "null"};
    gq_non_scalar_model = {"src", "test", "test-models", "gq_non_scalar"};
    gq_non_scalar_fitted_params
        = {"src", "test", "test-models", "gq_non_scalar_fitted_params.csv"};
    test_model = {"src", "test", "test-models", "test_model"};
  }
  std::vector<std::string> bern_gq_model;
  std::vector<std::string> bern_extra_model;
  std::vector<std::string> bern_data;
  std::vector<std::string> bern_fitted_params;
  std::vector<std::string> default_file_path;
  std::vector<std::string> dev_null_path;
  std::vector<std::string> gq_non_scalar_model;
  std::vector<std::string> gq_non_scalar_fitted_params;
  std::vector<std::string> test_model;
};

TEST_F(CmdStan, generate_quantities_good) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_non_scalar_good) {
  std::stringstream ss;
  ss << convert_model_path(gq_non_scalar_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(gq_non_scalar_fitted_params);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_no_data_arg) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_no_fitted_params_arg) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

  TEST_F(CmdStan, generate_quantities_missing_fitted_params) {
  std::stringstream ss;
  ss << convert_model_path(bern_extra_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_wrong_csv) {
  std::stringstream ss;
  ss << convert_model_path(test_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_csv_conflict) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(default_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(default_file_path);  // << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}
