#include <fstream>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <test/utility.hpp>

using cmdstan::test::convert_model_path;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
public:
  void SetUp() {
    std::vector<std::string> model_path;
    model_path.emplace_back("src");
    model_path.emplace_back("test");
    model_path.emplace_back("test-models");
    model_path.emplace_back("bernoulli_gq");

    std::vector<std::string> data_file_path;
    data_file_path.emplace_back("src");
    data_file_path.emplace_back("test");
    data_file_path.emplace_back("interface");
    data_file_path.emplace_back("bernoulli.data.json");

    std::vector<std::string> output_file_path;
    output_file_path.emplace_back("test");
    output_file_path.emplace_back("output.csv");

    base_command = convert_model_path(model_path)
      + " data file=" + convert_model_path(data_file_path);
  }

  std::string base_command;
};

TEST_F(CmdStan, generated_quantities) {
  std::vector<std::string> fitted_params_file_path;
  fitted_params_file_path.emplace_back("src");
  fitted_params_file_path.emplace_back("test");
  fitted_params_file_path.emplace_back("interface");
  fitted_params_file_path.emplace_back("bernoulli_fit.csv");

  run_command_output out = run_command(base_command
                                       + " generated_quantities"
                                       + " fitted_params=" + convert_model_path(fitted_params_file_path));
  ASSERT_EQ(0, out.err_code);
}
