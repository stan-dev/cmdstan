#include <test/utility.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <gtest/gtest.h>

using cmdstan::test::convert_model_path;
using cmdstan::test::parse_sample;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    multi_normal_model = {"src", "test", "test-models", "multi_normal_model"};
    multi_normal_mode_csv
        = {"src", "test", "test-models", "multi_normal_mode.csv"};
    multi_normal_mode_csv_bad_config
        = {"src", "test", "test-models", "multi_normal_mode_bad_config.csv"};
    multi_normal_mode_csv_bad_names
        = {"src", "test", "test-models", "multi_normal_mode_bad_names.csv"};
    multi_normal_mode_csv_bad_values
        = {"src", "test", "test-models", "multi_normal_mode_bad_values.csv"};
    multi_normal_mode_json
        = {"src", "test", "test-models", "multi_normal_mode.json"};
    default_file_path = {"src", "test", "test-models", "output.csv"};
    dev_null_path = {"/dev", "null"};
    wrong_csv = {"src", "test", "test-models", "bern_fitted_params.csv"};
    simple_jacobian_model
        = {"src", "test", "test-models", "simple_jacobian_model"};
    simple_jacobian_mode_json
        = {"src", "test", "test-models", "simple_jacobian_mode.json"};
    output1_csv = {"test", "tmp_output1.csv"};
    output2_csv = {"test", "tmp_output2.csv"};
    simplex_model = {"src", "test", "test-models", "simplex_model"};
    simplex_mode_csv = {"src", "test", "test-models", "simplex_mode.csv"};
    simplex_mode_bad_csv
        = {"src", "test", "test-models", "simplex_mode_bad.csv"};
  }
  std::vector<std::string> default_file_path;
  std::vector<std::string> dev_null_path;
  std::vector<std::string> simple_jacobian_model;
  std::vector<std::string> simple_jacobian_mode_json;
  std::vector<std::string> multi_normal_model;
  std::vector<std::string> multi_normal_mode_csv;
  std::vector<std::string> multi_normal_mode_json;
  std::vector<std::string> output1_csv;
  std::vector<std::string> output2_csv;
  std::vector<std::string> wrong_csv;
  std::vector<std::string> multi_normal_mode_csv_bad_config;
  std::vector<std::string> multi_normal_mode_csv_bad_names;
  std::vector<std::string> multi_normal_mode_csv_bad_values;
  std::vector<std::string> simplex_model;
  std::vector<std::string> simplex_mode_csv;
  std::vector<std::string> simplex_mode_bad_csv;
};

TEST_F(CmdStan, laplace_good) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode=" << convert_model_path(multi_normal_mode_csv);
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);

  ss.str(std::string());
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode=" << convert_model_path(multi_normal_mode_json);
  out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, laplace_no_mode_arg) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, laplace_wrong_mode_file) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode=" << convert_model_path(wrong_csv);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, laplace_missing_mode) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode="
     << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, laplace_bad_draws_arg) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode=" << convert_model_path(multi_normal_mode_csv)
     << " draws=0 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, laplace_bad_csv_file) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode="
     << convert_model_path(multi_normal_mode_csv_bad_config)
     << " draws=10 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);

  ss.str(std::string());
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode="
     << convert_model_path(multi_normal_mode_csv_bad_names) << " draws=10 2>&1";
  cmd = ss.str();
  out = run_command(cmd);
  ASSERT_TRUE(out.hasError);

  ss.str(std::string());
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=laplace mode="
     << convert_model_path(multi_normal_mode_csv_bad_values)
     << " draws=10 2>&1";
  cmd = ss.str();
  out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, laplace_jacobian_adjust) {
  std::stringstream ss;
  ss << convert_model_path(simple_jacobian_model) << " random seed=1234"
     << " method=laplace mode=" << convert_model_path(simple_jacobian_mode_json)
     << " output file=" << convert_model_path(output1_csv);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  std::vector<std::string> config;
  std::vector<std::string> header;
  std::vector<double> values1;
  parse_sample(convert_model_path(output1_csv), config, header, values1);
  double* ptr1 = &values1[0];
  Eigen::MatrixXd sample1  // 1000 draws, 1 variable == 3 outputs
      = Eigen::Map<Eigen::Matrix<double, 1000, 3, Eigen::RowMajor>>(ptr1);

  ss.str(std::string());
  ss << convert_model_path(simple_jacobian_model) << " random seed=1234"
     << " method=laplace mode=" << convert_model_path(simple_jacobian_mode_json)
     << " jacobian=0 "
     << " output file=" << convert_model_path(output2_csv);
  cmd = ss.str();
  out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  std::vector<double> values2;
  parse_sample(convert_model_path(output2_csv), config, header, values2);
  double* ptr2 = &values2[0];
  Eigen::MatrixXd sample2
      = Eigen::Map<Eigen::Matrix<double, 1000, 3, Eigen::RowMajor>>(ptr2);

  double sigma_est1 = (sample1.col(2).array() / sample1.rows()).sum();
  ASSERT_NEAR(3.1, sigma_est1, 0.15);

  double sigma_est2 = (sample2.col(2).array() / sample2.rows()).sum();
  ASSERT_NEAR(3.1, sigma_est2, 0.15);

  for (int n = 0; n < 1000; ++n) {
    ASSERT_NE(sample1.coeff(n, 0), sample2.coeff(n, 0));
    ASSERT_EQ(sample1.coeff(n, 1), sample2.coeff(n, 1));
    ASSERT_EQ(sample1.coeff(n, 2), sample2.coeff(n, 2));
  }
}

TEST_F(CmdStan, laplace_unconstrain) {
  std::stringstream ss;
  ss << convert_model_path(simplex_model) << " random seed=1234"
     << " method=laplace mode=" << convert_model_path(simplex_mode_csv)
     << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);

  ss.str(std::string());
  ss << convert_model_path(simplex_model) << " random seed=1234"
     << " method=laplace mode=" << convert_model_path(simplex_mode_bad_csv)
     << " 2>&1";
  cmd = ss.str();
  out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}
