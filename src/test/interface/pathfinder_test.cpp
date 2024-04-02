#include <test/utility.hpp>
#include <test/unit/util.hpp>
#include <fstream>
#include <gtest/gtest.h>

using cmdstan::test::convert_model_path;
using cmdstan::test::file_exists;
using cmdstan::test::parse_sample;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    dev_null_path = {"/dev", "null"};
    multi_normal_model = {"src", "test", "test-models", "multi_normal_model"};
    eight_schools_model = {"src", "test", "test-models", "eight_schools"};
    eight_schools_data
        = {"src", "test", "test-models", "eight_schools.data.json"};
    empty_model = {"src", "test", "test-models", "empty"};
    arg_output = {"test", "output"};
    arg_diags = {"test", "diagnostics"};
    output_csv = {"test", "output.csv"};
    output_json = {"test", "output.json"};
    output_diags = {"test", "diagnostics.json"};
    output_single_csv = {"test", "output_path_1.csv"};
    output_single_json = {"test", "output_path_1.json"};
  }

  void TearDown() {
    std::remove(convert_model_path(output_csv).c_str());
    std::remove(convert_model_path(output_json).c_str());
    std::remove(convert_model_path(output_diags).c_str());
    std::remove(convert_model_path(output_single_csv).c_str());
    std::remove(convert_model_path(output_single_json).c_str());
  }

  std::vector<std::string> dev_null_path;
  std::vector<std::string> multi_normal_model;
  std::vector<std::string> eight_schools_model;
  std::vector<std::string> eight_schools_data;
  std::vector<std::string> empty_model;
  std::vector<std::string> arg_output;
  std::vector<std::string> arg_diags;
  std::vector<std::string> output_csv;
  std::vector<std::string> output_json;
  std::vector<std::string> output_diags;
  std::vector<std::string> output_single_csv;
  std::vector<std::string> output_single_json;
};

TEST_F(CmdStan, pathfinder_defaults) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " method=pathfinder";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);

  std::fstream result_stream(convert_model_path(output_csv));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  EXPECT_EQ(1, count_matches("# Elapsed Time:", output));
  EXPECT_EQ(1, count_matches(" seconds (Pathfinders)", output));
  EXPECT_EQ(1, count_matches(" seconds (PSIS)", output));
  EXPECT_EQ(1, count_matches(" seconds (Total)", output));
  EXPECT_EQ(1, count_matches("save_single_paths = false (Default)", output));
  EXPECT_EQ(1, count_matches("num_paths = 4 (Default)", output));
}

TEST_F(CmdStan, pathfinder_40_draws) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " method=pathfinder num_psis_draws=40";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);

  std::fstream result_stream(convert_model_path(output_csv));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  EXPECT_EQ(1, count_matches("# Elapsed Time:", output));
  EXPECT_EQ(1, count_matches(" seconds (Pathfinders)", output));
  EXPECT_EQ(1, count_matches(" seconds (PSIS)", output));
  EXPECT_EQ(1, count_matches(" seconds (Total)", output));
  EXPECT_EQ(1, count_matches("num_psis_draws = 40", output));
  EXPECT_EQ(1, count_matches("num_paths = 4 (Default)", output));
  EXPECT_EQ(1, count_matches("save_single_paths = false (Default)", output));
}

TEST_F(CmdStan, pathfinder_single) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " method=pathfinder"
     << " num_paths=1";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  ASSERT_FALSE(file_exists(convert_model_path(output_json)));

  std::fstream result_stream(convert_model_path(output_csv));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  EXPECT_EQ(1, count_matches("Elapsed Time:", output));
  EXPECT_EQ(1, count_matches("seconds (Pathfinder)", output));
  EXPECT_EQ(1, count_matches("num_paths = 1", output));
  EXPECT_EQ(1, count_matches("save_single_paths = false (Default)", output));
}

bool is_whitespace(char c) { return c == ' ' || c == '\n'; }

TEST_F(CmdStan, pathfinder_save_single_default_num_paths) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " method=pathfinder save_single_paths=1";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  ASSERT_TRUE(file_exists(convert_model_path(output_csv)));
  ASSERT_FALSE(file_exists(convert_model_path(output_json)));
  ASSERT_TRUE(file_exists(convert_model_path(output_single_csv)));
  ASSERT_TRUE(file_exists(convert_model_path(output_single_json)));

  std::fstream single_csv_stream(convert_model_path(output_single_csv));
  std::stringstream result_sstream;
  result_sstream << single_csv_stream.rdbuf();
  single_csv_stream.close();
  std::string single_csv = result_sstream.str();
  EXPECT_EQ(1, count_matches("Elapsed Time:", single_csv));
  EXPECT_EQ(1, count_matches("seconds (Pathfinder)", single_csv));
  EXPECT_EQ(1, count_matches("save_single_paths = true", single_csv));

  std::fstream single_json_stream(convert_model_path(output_single_json));
  std::stringstream result_json_sstream;
  result_json_sstream << single_json_stream.rdbuf();
  single_json_stream.close();
  std::string single_json = result_json_sstream.str();
  ASSERT_FALSE(single_json.empty());

  ASSERT_TRUE(stan::test::is_valid_JSON(single_json));
  single_json.erase(
      std::remove_if(single_json.begin(), single_json.end(), is_whitespace),
      single_json.end());
  EXPECT_EQ(1, count_matches("\"1\":{\"iter\":1,", single_json));
}

TEST_F(CmdStan, pathfinder_save_single_num_paths_1) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " method=pathfinder"
     << " num_paths=1 save_single_paths=1";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  ASSERT_TRUE(file_exists(convert_model_path(output_csv)));
  ASSERT_TRUE(file_exists(convert_model_path(output_json)));
  ASSERT_FALSE(file_exists(convert_model_path(output_single_csv)));
  ASSERT_FALSE(file_exists(convert_model_path(output_single_json)));
}

TEST_F(CmdStan, pathfinder_save_single_num_paths_1_diag_file_arg) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " diagnostic_file=" << convert_model_path(arg_diags)
     << " method=pathfinder"
     << " num_paths=1 save_single_paths=1";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  ASSERT_TRUE(file_exists(convert_model_path(output_csv)));
  ASSERT_TRUE(file_exists(convert_model_path(output_diags)));
  ASSERT_FALSE(file_exists(convert_model_path(output_json)));
  ASSERT_FALSE(file_exists(convert_model_path(output_single_csv)));
  ASSERT_FALSE(file_exists(convert_model_path(output_single_json)));
}

TEST_F(CmdStan, pathfinder_save_single_num_paths_0_diag_file_arg) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " diagnostic_file=" << convert_model_path(arg_diags)
     << " method=pathfinder"
     << " num_paths=1 save_single_paths=0";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  ASSERT_TRUE(file_exists(convert_model_path(output_csv)));
  ASSERT_TRUE(file_exists(convert_model_path(output_diags)));
  ASSERT_FALSE(file_exists(convert_model_path(output_json)));
  ASSERT_FALSE(file_exists(convert_model_path(output_single_csv)));
  ASSERT_FALSE(file_exists(convert_model_path(output_single_json)));
}

TEST_F(CmdStan, pathfinder_num_paths_8) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " method=pathfinder"
     << " num_paths=8";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  ASSERT_TRUE(file_exists(convert_model_path(output_csv)));
  ASSERT_FALSE(file_exists(convert_model_path(output_single_csv)));

  std::fstream result_stream(convert_model_path(output_csv));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  EXPECT_EQ(1, count_matches(" seconds (Pathfinders)", output));
  EXPECT_EQ(1, count_matches(" seconds (PSIS)", output));
  EXPECT_EQ(1, count_matches("num_paths = 8", output));
}

TEST_F(CmdStan, pathfinder_lbfgs_iterations) {
  std::stringstream ss;
  ss << convert_model_path(eight_schools_model)
     << " data file=" << convert_model_path(eight_schools_data)
     << " random seed=12345"
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " method=pathfinder max_lbfgs_iters=3"
     << " save_single_paths=1";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  ASSERT_TRUE(file_exists(convert_model_path(output_csv)));
  ASSERT_TRUE(file_exists(convert_model_path(output_single_json)));

  std::fstream result_stream(convert_model_path(output_single_json));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  ASSERT_FALSE(output.empty());
  rapidjson::Document document;
  ASSERT_FALSE(document.Parse<0>(output.c_str()).HasParseError());
  output.erase(std::remove_if(output.begin(), output.end(), is_whitespace),
               output.end());
  EXPECT_EQ(1, count_matches("\"3\":{\"iter\":3,", output));
  EXPECT_EQ(0, count_matches("\"4\":{\"iter\":4,", output));
}

TEST_F(CmdStan, pathfinder_empty_model) {
  std::stringstream ss;
  ss << convert_model_path(empty_model) << " method=pathfinder";
  run_command_output out = run_command(ss.str());
  ASSERT_TRUE(out.hasError);
  EXPECT_EQ(1, count_matches("Model has 0 parameters", out.output));
}

TEST_F(CmdStan, pathfinder_too_many_PSIS_draws) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model) << " method=pathfinder"
     << " num_paths=1 num_draws=10 num_psis_draws=11";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  EXPECT_EQ(
      1, count_matches("Warning: Number of PSIS draws is larger", out.output));
}
