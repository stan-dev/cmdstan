#include <test/utility.hpp>
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <stdexcept>

using cmdstan::test::convert_model_path;
using cmdstan::test::multiple_command_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    bern_extra_model = {"src", "test", "test-models", "bern_extra_model"};
    bern_data = {"src", "test", "test-models", "bern.data.json"};
    bern_unconstrained_params_rdump
        = {"src", "test", "test-models", "bern_unconstrained_params.R"};
    bern_constrained_params_rdump
        = {"src", "test", "test-models", "bern_constrained_params.R"};
    bern_unconstrained_params_json
        = {"src", "test", "test-models", "bern_unconstrained_params.json"};
    bern_constrained_params_json
        = {"src", "test", "test-models", "bern_constrained_params.json"};
    bern_unconstrained_params_short
        = {"src", "test", "test-models", "bern_unconstrained_params_short.json"};
    bern_constrained_params_short
        = {"src", "test", "test-models", "bern_constrained_params_short.json"};
    dev_null_path = {"/dev", "null"};
  }
  std::vector<std::string> bern_extra_model;
  std::vector<std::string> bern_data;
  std::vector<std::string> bern_unconstrained_params_rdump;
  std::vector<std::string> bern_constrained_params_rdump;
  std::vector<std::string> bern_unconstrained_params_json;
  std::vector<std::string> bern_constrained_params_json;
  std::vector<std::string> bern_unconstrained_params_short;
  std::vector<std::string> bern_constrained_params_short;
  std::vector<std::string> dev_null_path;

};

TEST_F(CmdStan, log_prob_good_rdump) {
  std::stringstream ss;
  ss << convert_model_path(bern_extra_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=log_prob unconstrained_params="
     << convert_model_path(bern_unconstrained_params_rdump)
     << " constrained_params="
     << convert_model_path(bern_constrained_params_rdump);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, log_prob_good_json) {
  std::stringstream ss;
  ss << convert_model_path(bern_extra_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=log_prob unconstrained_params="
     << convert_model_path(bern_unconstrained_params_json)
     << " constrained_params="
     << convert_model_path(bern_constrained_params_json);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, log_prob_good_rdump_json) {
  std::stringstream ss;
  ss << convert_model_path(bern_extra_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=log_prob unconstrained_params="
     << convert_model_path(bern_unconstrained_params_rdump)
     << " constrained_params="
     << convert_model_path(bern_constrained_params_json);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, log_prob_no_params) {
  std::stringstream ss;
  ss << convert_model_path(bern_extra_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=log_prob";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, log_prob_no_data) {
  std::stringstream ss;
  ss << convert_model_path(bern_extra_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=log_prob unconstrained_params="
     << convert_model_path(bern_unconstrained_params_rdump)
     << " constrained_params="
     << convert_model_path(bern_constrained_params_json);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, log_prob_constrained_length) {
  std::stringstream ss;
  ss << convert_model_path(bern_extra_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=log_prob constrained_params="
     << convert_model_path(bern_constrained_params_short);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, log_prob_unconstrained_length) {
  std::stringstream ss;
  ss << convert_model_path(bern_extra_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=log_prob unconstrained_params="
     << convert_model_path(bern_unconstrained_params_short);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}
