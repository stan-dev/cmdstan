#include <test/utility.hpp>
#include <gtest/gtest.h>

using cmdstan::test::convert_model_path;
using cmdstan::test::file_exists;
using cmdstan::test::is_valid_JSON;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    multi_normal_model = convert_model_path(
        std::vector{"src", "test", "test-models", "multi_normal_model"});
    arg_output = convert_model_path(std::vector{"test", "output"});

    output_csv = convert_model_path(std::vector{"test", "output.csv"});
    output_json = convert_model_path(std::vector{"test", "output_config.json"});

    output_csv_multi
        = convert_model_path(std::vector{"test", "output_multi.csv"});
    output_json_multi
        = convert_model_path(std::vector{"test", "output_multi_config.json"});
  }

  void TearDown() {
    std::remove(output_csv.c_str());
    std::remove(output_json.c_str());
    std::remove(output_csv_multi.c_str());
    std::remove(output_json_multi.c_str());
  }

  std::string multi_normal_model;
  std::string arg_output;
  std::string output_csv;
  std::string output_json;

  std::string output_csv_multi;
  std::string output_json_multi;
};

TEST_F(CmdStan, config_json_output_valid) {
  std::stringstream ss;
  ss << multi_normal_model << " sample output file=" << arg_output
     << " save_cmdstan_config=1";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError) << out.output;
  ASSERT_TRUE(file_exists(output_csv));
  ASSERT_TRUE(file_exists(output_json));

  std::fstream json_in(output_json);
  std::stringstream result_json_sstream;
  result_json_sstream << json_in.rdbuf();
  json_in.close();
  std::string json = result_json_sstream.str();

  ASSERT_FALSE(json.empty());
  ASSERT_TRUE(is_valid_JSON(json));
}

TEST_F(CmdStan, config_json_output_valid_multi) {
  std::stringstream ss;
  ss << multi_normal_model
     << " sample num_chains=2 output file=" << output_csv_multi << ","
     << output_csv << " save_cmdstan_config=true";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError) << out.output;
  ASSERT_TRUE(file_exists(output_csv_multi));
  ASSERT_TRUE(file_exists(output_csv));
  ASSERT_TRUE(file_exists(output_json_multi));
  ASSERT_FALSE(file_exists(output_json));

  std::fstream json_in(output_json_multi);
  std::stringstream result_json_sstream;
  result_json_sstream << json_in.rdbuf();
  json_in.close();
  std::string json = result_json_sstream.str();

  ASSERT_FALSE(json.empty());
  ASSERT_TRUE(is_valid_JSON(json));
}

TEST_F(CmdStan, config_json_output_not_requested) {
  std::stringstream ss;
  ss << multi_normal_model << " sample output file=" << arg_output;
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  ASSERT_TRUE(file_exists(output_csv));
  ASSERT_FALSE(file_exists(output_json));
}
