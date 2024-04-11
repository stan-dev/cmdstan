#include <test/utility.hpp>
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <stdexcept>

using cmdstan::test::convert_model_path;
using cmdstan::test::multiple_command_separator;
using cmdstan::test::parse_sample;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    bern_log_prob_model = {"src", "test", "test-models", "bern_log_prob_model"};
    bern_gq_model = {"src", "test", "test-models", "bern_gq_model"};
    bern_data = {"src", "test", "test-models", "bern.data.json"};
    bern_unconstrained_params_rdump
        = {"src", "test", "test-models", "bern_unconstrained_params.R"};
    bern_constrained_params_rdump
        = {"src", "test", "test-models", "bern_constrained_params.R"};
    bern_unconstrained_params_json
        = {"src", "test", "test-models", "bern_unconstrained_params.json"};
    bern_constrained_params_json
        = {"src", "test", "test-models", "bern_constrained_params.json"};
    bern_constrained_params_csv
        = {"src", "test", "test-models", "bern_fitted_params.csv"};
    bern_unconstrained_params_short = {"src", "test", "test-models",
                                       "bern_unconstrained_params_short.json"};
    bern_constrained_params_short
        = {"src", "test", "test-models", "bern_constrained_params_short.json"};
    dev_null_path = {"/dev", "null"};
    test_output = {"test", "output.csv"};
    simplex_model = {"src", "test", "test-models", "simplex_model"};
    simplex_constrained_bad_csv
        = {"src", "test", "test-models", "simplex_mode_bad.csv"};
    simplex_constrained_csv
        = {"src", "test", "test-models", "simplex_mode.csv"};
  }
  std::vector<std::string> bern_log_prob_model;
  std::vector<std::string> bern_gq_model;
  std::vector<std::string> bern_data;
  std::vector<std::string> bern_unconstrained_params_rdump;
  std::vector<std::string> bern_constrained_params_rdump;
  std::vector<std::string> bern_unconstrained_params_json;
  std::vector<std::string> bern_constrained_params_json;
  std::vector<std::string> bern_constrained_params_csv;
  std::vector<std::string> bern_unconstrained_params_short;
  std::vector<std::string> bern_constrained_params_short;
  std::vector<std::string> dev_null_path;
  std::vector<std::string> test_output;
  std::vector<std::string> simplex_model;
  std::vector<std::string> simplex_constrained_csv;
  std::vector<std::string> simplex_constrained_bad_csv;
};

TEST_F(CmdStan, log_prob_uparams_rdump) {
  std::stringstream ss;
  ss << convert_model_path(bern_log_prob_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(test_output)
     << " method=log_prob unconstrained_params="
     << convert_model_path(bern_unconstrained_params_rdump);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  std::vector<std::string> config;
  std::vector<std::string> header;
  std::vector<double> values;
  parse_sample(convert_model_path(test_output), config, header, values);
  std::vector<std::string> names;
  boost::split(names, header[0], boost::is_any_of(","),
               boost::token_compress_on);
  ASSERT_EQ(values.size() % names.size(), 0);
  ASSERT_EQ(names[0].compare(0, 2, std::string("lp")), 0);
  ASSERT_EQ(names[1].compare(0, 2, std::string("g_")), 0);
}

TEST_F(CmdStan, log_prob_cparams_rdump) {
  std::stringstream ss;
  ss << convert_model_path(bern_log_prob_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(test_output)
     << " method=log_prob constrained_params="
     << convert_model_path(bern_constrained_params_rdump);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  std::vector<std::string> config;
  std::vector<std::string> header;
  std::vector<double> values;
  parse_sample(convert_model_path(test_output), config, header, values);
  std::vector<std::string> names;
  boost::split(names, header[0], boost::is_any_of(","),
               boost::token_compress_on);
  ASSERT_TRUE(values.size() % names.size() == 0);
}

TEST_F(CmdStan, log_prob_uparams_json) {
  std::stringstream ss;
  ss << convert_model_path(bern_log_prob_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(test_output)
     << " method=log_prob unconstrained_params="
     << convert_model_path(bern_unconstrained_params_json);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  std::vector<std::string> config;
  std::vector<std::string> header;
  std::vector<double> values;
  parse_sample(convert_model_path(test_output), config, header, values);
  std::vector<std::string> names;
  boost::split(names, header[0], boost::is_any_of(","),
               boost::token_compress_on);
  ASSERT_EQ(values.size() % names.size(), 0);
}

TEST_F(CmdStan, log_prob_cparams_json) {
  std::stringstream ss;
  ss << convert_model_path(bern_log_prob_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(test_output)
     << " method=log_prob constrained_params="
     << convert_model_path(bern_constrained_params_json);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  std::vector<std::string> config;
  std::vector<std::string> header;
  std::vector<double> values;
  parse_sample(convert_model_path(test_output), config, header, values);
  std::vector<std::string> names;
  boost::split(names, header[0], boost::is_any_of(","),
               boost::token_compress_on);
  ASSERT_EQ(values.size() % names.size(), 0);
}

TEST_F(CmdStan, log_prob_cparams_csv) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(test_output)
     << " method=log_prob constrained_params="
     << convert_model_path(bern_constrained_params_csv);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  // check that output CSV file has correct header row

  std::vector<std::string> config;
  std::vector<std::string> header;
  std::vector<double> values;
  parse_sample(convert_model_path(test_output), config, header, values);
  std::vector<std::string> names;
  boost::split(names, header[0], boost::is_any_of(","),
               boost::token_compress_on);
  ASSERT_EQ(values.size() % names.size(), 0);
  ASSERT_EQ(names[0].compare(0, 2, std::string("lp")), 0);
  ASSERT_EQ(names[1].compare(std::string("g_theta")), 0);
  ASSERT_EQ(values.size(), 2000);  // 1000 draws, 2 cols
}

TEST_F(CmdStan, log_prob_no_params) {
  std::stringstream ss;
  ss << convert_model_path(bern_log_prob_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=log_prob";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, log_prob_constrained_bad_input) {
  std::stringstream ss;
  ss << convert_model_path(bern_log_prob_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=log_prob constrained_params="
     << convert_model_path(bern_constrained_params_short);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, log_prob_constrained_simplex) {
  std::stringstream ss;
  ss << convert_model_path(simplex_model)
     << " output file=" << convert_model_path(test_output)
     << " method=log_prob constrained_params="
     << convert_model_path(simplex_constrained_csv);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  std::vector<std::string> config;
  std::vector<std::string> header;
  std::vector<double> values;
  parse_sample(convert_model_path(test_output), config, header, values);
  std::vector<std::string> names;
  boost::split(names, header[0], boost::is_any_of(","),
               boost::token_compress_on);
  // param is 3-array of 3-simplex, unconstrained params size == 6 + lp__
  ASSERT_EQ(names.size(), 7);
  ASSERT_EQ(values.size() % names.size(), 0);
}

TEST_F(CmdStan, log_prob_constrained_bad_simplex) {
  std::stringstream ss;
  ss << convert_model_path(simplex_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=log_prob constrained_params="
     << convert_model_path(simplex_constrained_bad_csv);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, log_prob_unconstrained_bad_input) {
  std::stringstream ss;
  ss << convert_model_path(bern_log_prob_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=log_prob unconstrained_params="
     << convert_model_path(bern_unconstrained_params_short);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, log_prob_conflict) {
  std::stringstream ss;
  ss << convert_model_path(bern_log_prob_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(test_output)
     << " method=log_prob constrained_params="
     << convert_model_path(test_output);  // << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}
