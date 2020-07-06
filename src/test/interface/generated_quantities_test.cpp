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
    model_path = {"src", "test", "test-models", "gq_model"};
    data_file_path = {"src", "test", "test-models", "gq_model.data.json"};
    model_path_2 = {"src", "test", "test-models", "test_model"};
    model_path_non_scalar_gq = {"src", "test", "test-models", "gq_non_scalar"};
    output_file_path = {"/dev", "null"};
    fitted_params_file_path
        = {"src", "test", "test-models", "gq_model_output.csv"};
    fitted_params_file_path_2
        = {"src", "test", "test-models", "test_model_output.csv"};
    fitted_params_file_path_empty = {"src", "test", "test-models", "empty.csv"};
    fitted_params_non_scalar_gq
        = {"src", "test", "test-models", "gq_non_scalar.csv"};
    default_file_path = {"src", "test", "test-models", "output.csv"};
  }

  std::vector<std::string> model_path;
  std::vector<std::string> data_file_path;
  std::vector<std::string> model_path_2;
  std::vector<std::string> model_path_non_scalar_gq;
  std::vector<std::string> output_file_path;
  std::vector<std::string> fitted_params_file_path;
  std::vector<std::string> fitted_params_file_path_2;
  std::vector<std::string> fitted_params_file_path_empty;
  std::vector<std::string> fitted_params_non_scalar_gq;
  std::vector<std::string> default_file_path;
};

TEST_F(CmdStan, generate_quantities_good) {
  std::stringstream ss;
  ss << convert_model_path(model_path)
     << " data file=" << convert_model_path(data_file_path)
     << " output file=" << convert_model_path(output_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(fitted_params_file_path);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_non_scalar_good) {
  std::stringstream ss;
  ss << convert_model_path(model_path_non_scalar_gq)
     << " output file=" << convert_model_path(output_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(fitted_params_non_scalar_gq);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_bad_nodata) {
  std::stringstream ss;
  ss << convert_model_path(model_path)
     << " output file=" << convert_model_path(output_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(fitted_params_file_path_empty) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_bad_no_gqs) {
  std::stringstream ss;
  ss << convert_model_path(model_path_2)
     << " output file=" << convert_model_path(output_file_path)
     << " method=generate_quantities "
     << " fitted_params=" << convert_model_path(fitted_params_file_path_2)
     << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_wrong_csv) {
  std::stringstream ss;
  ss << convert_model_path(model_path)
     << " data file=" << convert_model_path(data_file_path)
     << " output file=" << convert_model_path(output_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(fitted_params_file_path_2) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_wrong_csv_2) {
  std::stringstream ss;
  ss << convert_model_path(model_path_2)
     << " data file=" << convert_model_path(data_file_path)
     << " output file=" << convert_model_path(output_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(fitted_params_file_path) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_csv_conflict) {
  std::stringstream ss;
  ss << convert_model_path(model_path)
     << " data file=" << convert_model_path(data_file_path)
     << " output file=" << convert_model_path(default_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(default_file_path);  // << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}
