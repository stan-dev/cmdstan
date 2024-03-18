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
    dev_null = {"/dev", "null"};
    multi_normal_model = {"src", "test", "test-models", "multi_normal_model"};
    simplex_model = {"src", "test", "test-models", "simplex_model"};
    output_csv = {"test", "output.csv"};
    output_metric = {"test", "output_metric.json"};
    output_metric_4 = {"test", "output_metric_4.json"};
  }

  void TearDown() {
    std::remove(convert_model_path(output_csv).c_str());
    std::remove(convert_model_path(output_metric).c_str());
  }

  std::vector<std::string> default_file;
  std::vector<std::string> dev_null;
  std::vector<std::string> multi_normal_model;
  std::vector<std::string> output_csv;
  std::vector<std::string> output_metric;
  std::vector<std::string> output_metric_4;
  std::vector<std::string> simplex_model;
};

TEST_F(CmdStan, save_diag_metric) {
  std::stringstream ss;
  ss << convert_model_path(simplex_model) << " random seed=1234"
     << " method=sample adapt save_metric=1"
     << " output file=" << convert_model_path(output_csv) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  ASSERT_TRUE(file_exists(convert_model_path(output_metric)));

  std::fstream result_stream(convert_model_path(output_metric));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string metric = result_sstream.str();
  ASSERT_TRUE(stan::test::is_valid_JSON(metric));
  EXPECT_EQ(count_matches("stepsize", metric), 1);
  EXPECT_EQ(count_matches("inv_metric", metric), 1);
  EXPECT_EQ(count_matches("[", metric), 1);  // diagonal metric
}

TEST_F(CmdStan, save_dense_metric) {
  std::stringstream ss;
  ss << convert_model_path(simplex_model) << " random seed=1234"
     << " method=sample adapt save_metric=1 algorithm=hmc metric=dense_e"
     << " output file=" << convert_model_path(output_csv) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  ASSERT_TRUE(file_exists(convert_model_path(output_metric)));

  std::fstream result_stream(convert_model_path(output_metric));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string metric = result_sstream.str();
  ASSERT_TRUE(stan::test::is_valid_JSON(metric));
  EXPECT_EQ(count_matches("stepsize", metric), 1);
  EXPECT_EQ(count_matches("inv_metric", metric), 1);
  EXPECT_EQ(count_matches("[", metric), 7);  // dense metric
}

TEST_F(CmdStan, save_unit_metric) {
  std::stringstream ss;
  ss << convert_model_path(simplex_model) << " random seed=1234"
     << " method=sample adapt save_metric=1 algorithm=hmc metric=unit_e"
     << " output file=" << convert_model_path(output_csv) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  ASSERT_TRUE(file_exists(convert_model_path(output_metric)));

  std::fstream result_stream(convert_model_path(output_metric));
  std::stringstream result_sstream;
  result_sstream << result_stream.rdbuf();
  result_stream.close();
  std::string metric = result_sstream.str();
  ASSERT_TRUE(stan::test::is_valid_JSON(metric));
  EXPECT_EQ(count_matches("stepsize", metric), 1);
  EXPECT_EQ(count_matches("inv_metric", metric), 1);
  EXPECT_EQ(count_matches("[", metric), 1);  // unit metric is diagonal
}

TEST_F(CmdStan, save_metric_no_adapt) {
  std::stringstream ss;
  ss << convert_model_path(simplex_model) << " random seed=1234"
     << " method=sample adapt engaged=0 save_metric=1 "
     << " output file=" << convert_model_path(output_csv) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}
