#include <fstream>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <test/utility.hpp>

using cmdstan::test::convert_model_path;
using cmdstan::test::multiple_command_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
public:
  void SetUp() {
    model_path = {"src", "test", "test-models", "gq_model"};
    data_file_path = {"src", "test", "test-models", "gq_model.data.json"};
    model_path_2 = { "src", "test", "test-models", "test_model"};
    output_file_path = { "/dev", "null"};
    fitted_params_file_path = {"src", "test", "test-models", "gq_model_output.csv"};
    fitted_params_file_path_2 = {"src", "test", "test-models", "test_model_output.csv"};
    fitted_params_file_path_empty = {"src", "test", "test-models", "empty.csv"};
  }

  std::vector<std::string> model_path;
  std::vector<std::string> data_file_path;
  std::vector<std::string> model_path_2;
  std::vector<std::string> output_file_path;
  std::vector<std::string> fitted_params_file_path;
  std::vector<std::string> fitted_params_file_path_2;
  std::vector<std::string> fitted_params_file_path_empty;

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

TEST_F(CmdStan, generate_quantities_bad_nodata) {
  std::stringstream ss;
  ss << convert_model_path(model_path)
     << " output file=" << convert_model_path(output_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(fitted_params_file_path_empty)
     << " 2>&1";
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
     << convert_model_path(fitted_params_file_path_2)
     << " 2>&1";
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
     << convert_model_path(fitted_params_file_path)
     << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}
