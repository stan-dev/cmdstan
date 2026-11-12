#include <test/utility.hpp>
#include <fstream>
#include <gtest/gtest.h>

using cmdstan::test::convert_model_path;
using cmdstan::test::count_matches;
using cmdstan::test::file_exists;
using cmdstan::test::is_valid_JSON;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStanTDJson : public testing::Test {
 public:
  void SetUp() {
    gq_non_scalar_model = {"src", "test", "test-models", "gq_non_scalar"};
    output_json = {"test", "transformed_data.json"};
  }

  void TearDown() { std::remove(convert_model_path(output_json).c_str()); }

  std::vector<std::string> gq_non_scalar_model;
  std::vector<std::string> output_json;
};

TEST_F(CmdStanTDJson, save) {
  std::stringstream ss;
  ss << convert_model_path(gq_non_scalar_model) << " random seed=1234"
     << " method=sample output transformed_data="
     << convert_model_path(output_json) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  ASSERT_TRUE(file_exists(convert_model_path(output_json)));

  std::fstream result_stream(convert_model_path(output_json));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string transformed_data = result_sstream.str();
  ASSERT_TRUE(is_valid_JSON(transformed_data));

  EXPECT_EQ(count_matches("X", transformed_data), 1);
  EXPECT_EQ(count_matches("y", transformed_data), 1);
  EXPECT_EQ(count_matches("N", transformed_data), 1);
  EXPECT_EQ(count_matches("K", transformed_data), 1);
}
