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
    model_path.emplace_back("src");
    model_path.emplace_back("test");
    model_path.emplace_back("test-models");
    model_path.emplace_back("gq_model");

    data_file_path.emplace_back("src");
    data_file_path.emplace_back("test");
    data_file_path.emplace_back("test-models");
    data_file_path.emplace_back("gq_model.data.json");

    model_path_2.emplace_back("src");
    model_path_2.emplace_back("test");
    model_path_2.emplace_back("test-models");
    model_path_2.emplace_back("test_model");

    output_file_path.emplace_back("/dev");
    output_file_path.emplace_back("null");

    fitted_params_file_path.emplace_back("src");
    fitted_params_file_path.emplace_back("test");
    fitted_params_file_path.emplace_back("test-models");
    fitted_params_file_path.emplace_back("gq_model_output.csv");

    fitted_params_file_path_2.emplace_back("src");
    fitted_params_file_path_2.emplace_back("test");
    fitted_params_file_path_2.emplace_back("test-models");
    fitted_params_file_path_2.emplace_back("test_model_output.csv");

    fitted_params_file_path_empty.emplace_back("src");
    fitted_params_file_path_empty.emplace_back("test");
    fitted_params_file_path_empty.emplace_back("test-models");
    fitted_params_file_path_empty.emplace_back("empty.csv");
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
  ASSERT_EQ(0, out.err_code);
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
  ASSERT_EQ(70, out.err_code);
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
  ASSERT_EQ(78, out.err_code);  # stan::services::error_codes CONFIG
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
  ASSERT_EQ(70, out.err_code);
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
  ASSERT_EQ(70, out.err_code);
}
