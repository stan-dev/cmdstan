#include <test/utility.hpp>
#include <rapidjson/document.h>
#include <fstream>
#include <gtest/gtest.h>

using cmdstan::test::convert_model_path;
using cmdstan::test::count_matches;
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
    arg_output = {"test", "output"};
    arg_diags = {"test", "output"};
    output_csv = {"test", "output.csv"};
    output_save_single = {"test", "output_path_1.csv"};
    output_save_single_diag = {"test", "output_path_1.json"};
    output_save_single_diag_1 = {"test", "output.json"};
  }

  void TearDown() {
    std::remove(convert_model_path(output_csv).c_str());
    std::remove(convert_model_path(output_save_single).c_str());
    std::remove(convert_model_path(output_save_single_diag).c_str());
    std::remove(convert_model_path(output_save_single_diag_1).c_str());
  }

  std::vector<std::string> dev_null_path;
  std::vector<std::string> multi_normal_model;
  std::vector<std::string> eight_schools_model;
  std::vector<std::string> eight_schools_data;
  std::vector<std::string> arg_output;
  std::vector<std::string> arg_diags;
  std::vector<std::string> output_csv;
  std::vector<std::string> output_save_single;
  std::vector<std::string> output_save_single_diag;
  std::vector<std::string> output_save_single_diag_1;
};

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
  EXPECT_EQ(1, count_matches("save_single_paths = 0 (Default)", output));
}

TEST_F(CmdStan, pathfinder_save_single_paths) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " method=pathfinder save_single_paths=1";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  std::fstream single_csv_stream(convert_model_path(output_save_single));
  std::stringstream result_sstream;
  result_sstream << single_csv_stream.rdbuf();
  single_csv_stream.close();
  std::string output_csv = result_sstream.str();
  EXPECT_EQ(1, count_matches("# Elapsed Time:", output_csv));
  EXPECT_EQ(1, count_matches(" seconds (Pathfinder)", output_csv));
  EXPECT_EQ(1, count_matches("save_single_paths = 1", output_csv));

  std::fstream single_json_stream(convert_model_path(output_save_single_diag));
  std::stringstream result_json_sstream;
  result_json_sstream << single_json_stream.rdbuf();
  single_json_stream.close();
  std::string output_json = result_json_sstream.str();
  ASSERT_FALSE(output_json.empty());

  rapidjson::Document document;
  ASSERT_FALSE(document.Parse<0>(output_json.c_str()).HasParseError());
  EXPECT_EQ(1, count_matches("\"1\" : {\"iter\" : 1,", output_json));
}

TEST_F(CmdStan, pathfinder_single) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " method=pathfinder"
     << " num_paths=1";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);

  std::fstream result_stream(convert_model_path(output_csv));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  EXPECT_EQ(1, count_matches("# Elapsed Time:", output));
  EXPECT_EQ(1, count_matches(" seconds (Pathfinder)", output));
  EXPECT_EQ(1, count_matches("num_paths = 1", output));
  EXPECT_EQ(1, count_matches("save_single_paths = 0 (Default)", output));
}

TEST_F(CmdStan, pathfinder_single_good_plus) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " method=pathfinder"
     << " num_paths=1"
     << " save_single_paths=1";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);

  std::fstream result_stream(convert_model_path(output_csv));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  EXPECT_EQ(1, count_matches("# Elapsed Time:", output));
  EXPECT_EQ(1, count_matches(" seconds (Pathfinder)", output));
  EXPECT_EQ(1, count_matches("num_paths = 1", output));
  EXPECT_EQ(1, count_matches("save_single_paths = 1", output));

  ASSERT_FALSE(file_exists(convert_model_path(output_save_single)));
}

TEST_F(CmdStan, pathfinder_num_paths_8) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " method=pathfinder"
     << " num_paths=8";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
  std::vector<std::string> test_psis_path = {"test", "output.csv"};
  std::fstream result_stream(convert_model_path(test_psis_path));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  EXPECT_EQ(1, count_matches("num_paths = 8", output));
  EXPECT_EQ(1, count_matches("# Elapsed Time:", output));
  EXPECT_EQ(1, count_matches(" seconds (Pathfinders)", output));
}

// TEST_F(CmdStan, pathfinder_diagnostic_json) {
//   std::stringstream ss;
//   ss << convert_model_path(multi_normal_model)
//      << " output refresh=0 file=" << convert_model_path(arg_output)
//      << " diagnostic_file=" << convert_model_path(arg_diags)
//      << " method=pathfinder";
//   run_command_output out = run_command(ss.str());
//   ASSERT_FALSE(out.hasError);

//   std::fstream result_stream(convert_model_path(output_diags));
//   std::stringstream result_sstream;
//   result_sstream << result_stream.rdbuf();
//   result_stream.close();
//   std::string output = result_sstream.str();
//   ASSERT_FALSE(output.empty());
//   rapidjson::Document document;
//   ASSERT_FALSE(document.Parse<0>(output.c_str()).HasParseError());
// }

// TEST_F(CmdStan, pathfinder_lbfgs_iterations) {
//   std::stringstream ss;
//   ss << convert_model_path(eight_schools_model)
//      << " data file=" << convert_model_path(eight_schools_data)
//      << " random seed=12345"
//      << " output refresh=0 file=" << convert_model_path(arg_output)
//      << " diagnostic_file=" << convert_model_path(arg_diags)
//      << " method=pathfinder max_lbfgs_iters=3";
//   run_command_output out = run_command(ss.str());
//   ASSERT_FALSE(out.hasError);

//   std::fstream result_stream(convert_model_path(output_diags));
//   std::stringstream result_sstream;
//   result_sstream << result_stream.rdbuf();
//   result_stream.close();
//   std::string output = result_sstream.str();
//   ASSERT_FALSE(output.empty());
//   rapidjson::Document document;
//   ASSERT_FALSE(document.Parse<0>(output.c_str()).HasParseError());
//   EXPECT_EQ(1, count_matches("\"3\" : {\"iter\" : 3,", output));
//   EXPECT_EQ(0, count_matches("\"4\" : {\"iter\" : 4,", output));
// }

TEST_F(CmdStan, pathfinder_num_paths_draws) {
  std::stringstream ss;
  ss << convert_model_path(eight_schools_model)
     << " data file=" << convert_model_path(eight_schools_data)
     << " output refresh=0 file=" << convert_model_path(arg_output)
     << " method=pathfinder num_draws=10 num_paths=2";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);

  std::fstream result_stream(convert_model_path(output_csv));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string output = result_sstream.str();
  ASSERT_FALSE(output.empty());
  EXPECT_EQ(1, count_matches("num_paths = 2", output));
  EXPECT_EQ(1, count_matches("num_draws = 10", output));
}
