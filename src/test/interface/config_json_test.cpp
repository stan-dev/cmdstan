#include <gtest/gtest.h>
#include <test/unit/util.hpp>
#include <test/utility.hpp>

using cmdstan::test::convert_model_path;
using cmdstan::test::file_exists;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    multi_normal_model = {"src", "test", "test-models", "multi_normal_model"};
    arg_output = {"test", "output"};
    output_csv = {"test", "output.csv"};
    output_json = {"test", "output_config.json"};
  }

  void TearDown() {
    std::remove(convert_model_path(output_csv).c_str());
    std::remove(convert_model_path(output_json).c_str());
  }

  std::vector<std::string> multi_normal_model;
  std::vector<std::string> arg_output;
  std::vector<std::string> output_csv;
  std::vector<std::string> output_json;
};

TEST_F(CmdStan, config_json_output_valid) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " sample output file=" << convert_model_path(arg_output)
     << " save_cmdstan_config=1";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError) << out.output;
  ASSERT_TRUE(file_exists(convert_model_path(output_csv)));
  ASSERT_TRUE(file_exists(convert_model_path(output_json)));

  std::fstream json_in(convert_model_path(output_json));
  std::stringstream result_json_sstream;
  result_json_sstream << json_in.rdbuf();
  json_in.close();
  std::string json = result_json_sstream.str();

  ASSERT_FALSE(json.empty());
  ASSERT_TRUE(stan::test::is_valid_JSON(json));
}

TEST_F(CmdStan, config_json_output_not_requested) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " sample output file=" << convert_model_path(arg_output);
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  ASSERT_TRUE(file_exists(convert_model_path(output_csv)));
  ASSERT_FALSE(file_exists(convert_model_path(output_json)));
}
