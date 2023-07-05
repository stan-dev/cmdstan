#include <test/utility.hpp>
#include <rapidjson/document.h>
#include <gtest/gtest.h>

using cmdstan::test::convert_model_path;
using cmdstan::test::count_matches;
using cmdstan::test::parse_sample;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    dev_null_path = {"/dev", "null"};
    multi_normal_model = {"src", "test", "test-models", "multi_normal_model"};
    test_arg_output = {"test", "tmp_pf"};
    test_arg_diags = {"test", "tmp_pf"};
    test_result_csv = {"test", "tmp_pf_pathfinder.csv"};
    test_result_single = {"test", "tmp_pf_1.csv"};
    test_result_diags = {"test", "tmp_pf_1.json"};
  }
  std::vector<std::string> dev_null_path;
  std::vector<std::string> multi_normal_model;
  std::vector<std::string> test_arg_output;
  std::vector<std::string> test_arg_diags;
  std::vector<std::string> test_result_csv;
  std::vector<std::string> test_result_single;
  std::vector<std::string> test_result_diags;
};

TEST_F(CmdStan, pathfinder_good) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(test_arg_output)
     << " method=pathfinder";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);

  std::fstream result_stream(convert_model_path(test_result_csv));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  EXPECT_EQ(1, count_matches("# Elapsed Time:", output));
  EXPECT_EQ(1, count_matches(" seconds (Pathfinders)", output));
  EXPECT_EQ(1, count_matches(" seconds (PSIS)", output));
  EXPECT_EQ(1, count_matches(" seconds (Total)", output));

  result_sstream.str(std::string());
  std::fstream single_stream(convert_model_path(test_result_single));
  result_sstream << single_stream.rdbuf();
  single_stream.close();
  output = result_sstream.str();
  EXPECT_EQ(1, count_matches("# Elapsed Time:", output));
  EXPECT_EQ(1, count_matches(" seconds (Pathfinder)", output));
}

TEST_F(CmdStan, pathfinder_single_good) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(test_arg_output)
     << " method=pathfinder"
     << " num_paths=1";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);

  std::vector<std::string> test_result_1path = {"test", "tmp_pf.csv"};
  std::fstream result_stream(convert_model_path(test_result_1path));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  EXPECT_EQ(1, count_matches("# Elapsed Time:", output));
  EXPECT_EQ(1, count_matches(" seconds (Pathfinder)", output));
}

TEST_F(CmdStan, pathfinder_multi_good) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(test_arg_output)
     << " method=pathfinder"
     << " num_paths=8";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  std::vector<std::string> test_result_8path = {"test", "tmp_pf_8.csv"};
  std::fstream result_stream(convert_model_path(test_result_8path));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  EXPECT_EQ(1, count_matches("# Elapsed Time:", output));
  EXPECT_EQ(1, count_matches(" seconds (Pathfinder)", output));
}

TEST_F(CmdStan, pathfinder_diagnostic_json) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(test_arg_output)
     << " diagnostic_file=" << convert_model_path(test_arg_diags)
     << " method=pathfinder";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);

  std::fstream result_stream(convert_model_path(test_result_diags));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  ASSERT_FALSE(output.empty());
  rapidjson::Document document;
  ASSERT_FALSE(document.Parse<0>(output.c_str()).HasParseError());
}
