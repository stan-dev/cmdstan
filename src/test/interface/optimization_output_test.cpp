#include <test/utility.hpp>
#include <stan/mcmc/chains.hpp>
#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>
#include <fstream>

using cmdstan::test::convert_model_path;
using cmdstan::test::idx_first_match;
using cmdstan::test::parse_sample;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

// outimization_model - matrix of 4 normals
// [[1, 10000], [100, 1000000]] - column major:
// lp__, y.1.1, y.2.1, y.1.2, y.2.2
// 0, 1, 100, 10000, 1e+06

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    optimization_model = {"src", "test", "test-models", "optimization_output"};
    simple_jacobian_model
        = {"src", "test", "test-models", "simple_jacobian_model"};

    output1_csv = {"test", "output1.csv"};
    output2_csv = {"test", "output2.csv"};
    algorithm = "algorithm = ";
    jacobian = "jacobian = ";
  }

  std::vector<std::string> optimization_model;
  std::vector<std::string> simple_jacobian_model;
  std::vector<std::string> output1_csv;
  std::vector<std::string> output2_csv;
  std::string algorithm;
  std::string jacobian;
};

TEST_F(CmdStan, optimize_default) {
  std::stringstream ss;
  ss << convert_model_path(optimization_model)
     << " output file=" << convert_model_path(output1_csv)
     << " method=optimize 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_EQ(0, out.err_code);

  std::vector<std::string> config;
  std::vector<std::string> header;
  std::vector<double> values;
  parse_sample(convert_model_path(output1_csv), config, header, values);

  int algo_idx = idx_first_match(config, algorithm);
  EXPECT_NE(algo_idx, -1);
  EXPECT_TRUE(boost::contains(config[algo_idx], "(Default)"));

  int jacobian_idx = idx_first_match(config, jacobian);
  EXPECT_NE(jacobian_idx, -1);
  EXPECT_TRUE(boost::contains(config[jacobian_idx], "= false (Default)"));

  ASSERT_NEAR(0, values[0], 0.00001);
  EXPECT_FLOAT_EQ(1, values[1]);
  EXPECT_FLOAT_EQ(100, values[2]);
  EXPECT_FLOAT_EQ(10000, values[3]);
  EXPECT_FLOAT_EQ(1000000, values[4]);
}

TEST_F(CmdStan, optimize_bfgs) {
  std::stringstream ss;
  ss << convert_model_path(optimization_model)
     << " output file=" << convert_model_path(output1_csv)
     << " method=optimize algorithm=bfgs 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_EQ(0, out.err_code);

  std::vector<std::string> config;
  std::vector<std::string> header;
  std::vector<double> values;
  parse_sample(convert_model_path(output1_csv), config, header, values);

  int algo_idx = idx_first_match(config, algorithm);
  EXPECT_NE(algo_idx, -1);
  EXPECT_TRUE(boost::contains(config[algo_idx], "bfgs"));
  EXPECT_FALSE(boost::contains(config[algo_idx], "lbfgs"));

  ASSERT_NEAR(0, values[0], 0.00001);
  EXPECT_FLOAT_EQ(1, values[1]);
  EXPECT_FLOAT_EQ(100, values[2]);
  EXPECT_FLOAT_EQ(10000, values[3]);
  EXPECT_FLOAT_EQ(1000000, values[4]);
}

TEST_F(CmdStan, optimize_lbfgs) {
  std::stringstream ss;
  ss << convert_model_path(optimization_model)
     << " output file=" << convert_model_path(output1_csv)
     << " method=optimize algorithm=lbfgs 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_EQ(0, out.err_code);

  std::vector<std::string> config;
  std::vector<std::string> header;
  std::vector<double> values;
  parse_sample(convert_model_path(output1_csv), config, header, values);

  int algo_idx = idx_first_match(config, algorithm);
  EXPECT_NE(algo_idx, -1);
  EXPECT_TRUE(boost::contains(config[algo_idx], "lbfgs"));

  ASSERT_NEAR(0, values[0], 0.00001);
  EXPECT_FLOAT_EQ(1, values[1]);
  EXPECT_FLOAT_EQ(100, values[2]);
  EXPECT_FLOAT_EQ(10000, values[3]);
  EXPECT_FLOAT_EQ(1000000, values[4]);
}

TEST_F(CmdStan, optimize_newton) {
  std::stringstream ss;
  ss << convert_model_path(optimization_model)
     << " output file=" << convert_model_path(output1_csv)
     << " method=optimize algorithm=newton 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_EQ(0, out.err_code);

  std::vector<std::string> config;
  std::vector<std::string> header;
  std::vector<double> values;
  parse_sample(convert_model_path(output1_csv), config, header, values);

  int algo_idx = idx_first_match(config, algorithm);
  EXPECT_NE(algo_idx, -1);
  EXPECT_TRUE(boost::contains(config[algo_idx], "newton"));

  ASSERT_NEAR(0, values[0], 0.00001);
  EXPECT_FLOAT_EQ(1, values[1]);
  EXPECT_FLOAT_EQ(100, values[2]);
  EXPECT_FLOAT_EQ(10000, values[3]);
  EXPECT_FLOAT_EQ(1000000, values[4]);
}

TEST_F(CmdStan, optimize_jacobian_adjust) {
  std::stringstream ss;
  ss << convert_model_path(simple_jacobian_model) << " random seed=1234"
     << " output file=" << convert_model_path(output1_csv)
     << " method=optimize 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  std::vector<std::string> config1;
  std::vector<std::string> header;
  std::vector<double> values1;
  parse_sample(convert_model_path(output1_csv), config1, header, values1);

  int jacobian_idx = idx_first_match(config1, jacobian);
  EXPECT_NE(jacobian_idx, -1);
  EXPECT_TRUE(boost::contains(config1[jacobian_idx], "= false (Default)"));

  ASSERT_NEAR(0, values1[0], 0.00001);
  ASSERT_NEAR(3, values1[1], 0.01);

  ss.str(std::string());
  ss << convert_model_path(simple_jacobian_model) << " random seed=1234"
     << " output file=" << convert_model_path(output2_csv)
     << " method=optimize jacobian=1 2>&1";
  cmd = ss.str();
  out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  std::vector<std::string> config2;
  std::vector<double> values2;
  parse_sample(convert_model_path(output2_csv), config2, header, values2);

  jacobian_idx = idx_first_match(config2, jacobian);
  EXPECT_NE(jacobian_idx, -1);
  EXPECT_TRUE(boost::contains(config2[jacobian_idx], "= true"));

  ASSERT_NEAR(3.3, values2[1], 0.01);
}
