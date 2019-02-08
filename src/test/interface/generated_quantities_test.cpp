#include <fstream>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <test/utility.hpp>

using cmdstan::test::convert_model_path;
using cmdstan::test::multiple_command_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

TEST(CmdStan, generate_quantities_good) {

  std::vector<std::string> model_path;
  model_path.emplace_back("src");
  model_path.emplace_back("test");
  model_path.emplace_back("test-models");
  model_path.emplace_back("gq_model");

  std::vector<std::string> data_file_path;
  data_file_path.emplace_back("src");
  data_file_path.emplace_back("test");
  data_file_path.emplace_back("test-models");
  data_file_path.emplace_back("gq_model.data.json");

  std::vector<std::string> output_file_path;
  output_file_path.emplace_back("src");
  output_file_path.emplace_back("test");
  output_file_path.emplace_back("test");
  output_file_path.emplace_back("output.csv");

  std::vector<std::string> fitted_params_file_path;
  fitted_params_file_path.emplace_back("src");
  fitted_params_file_path.emplace_back("test");
  fitted_params_file_path.emplace_back("test-models");
  fitted_params_file_path.emplace_back("gq_model_fit.csv");

  std::stringstream ss;
  ss << convert_model_path(model_path)
     << " data file=" << convert_model_path(data_file_path);
  std::string base_command = ss.str();
  std::cout << base_command << std::endl;
  run_command_output out =
      run_command(base_command + " generate_quantities" + " fitted_params=" +
                  convert_model_path(fitted_params_file_path));
  ASSERT_EQ(0, out.err_code);
}
