#include <test/utility.hpp>
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <stdexcept>
#include <stan/io/stan_csv_reader.hpp>

using cmdstan::test::convert_model_path;
using cmdstan::test::multiple_command_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    bern_model = {"src", "test", "test-models", "bern_gq_model"};
    bern_data = {"src", "test", "test-models", "bern.data.json"};
    init_data = {"src", "test", "test-models", "bern_init.json"};
    init2_data = {"src", "test", "test-models", "bern_init2.json"};
    init3_data = {"src", "test", "test-models", "bern_init2.R"};
    init_bad_data = {"src", "test", "test-models", "bern_init_bad.json"};
    dev_null_path = {"/dev", "null"};
  }
  std::vector<std::string> bern_model;
  std::vector<std::string> dev_null_path;
  std::vector<std::string> bern_data;
  std::vector<std::string> init_data;
  std::vector<std::string> init2_data;
  std::vector<std::string> init3_data;
  std::vector<std::string> init_bad_data;
};

TEST_F(CmdStan, multi_chain_single_init_file_good) {
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " init=" << convert_model_path(init_data)
     << " method=sample num_chains=2";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, multi_chain_multi_init_file_good) {
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " init=" << convert_model_path(init2_data)
     << " method=sample num_chains=4";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, multi_chain_multi_init_file_id_good) {
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " init=" << convert_model_path(init2_data) << " id=2"
     << " method=sample num_chains=2";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError) << out.output;
}

TEST_F(CmdStan, multi_chain_multi_init_file_id_bad) {
  // this will start by requesting ..._4.json, which doesn't exist
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " init=" << convert_model_path(init2_data) << " id=4"
     << " method=sample num_chains=3";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, multi_chain_multi_init_file_actually_used) {
  // the second chain has a bad init value
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " init=" << convert_model_path(init_bad_data)
     << " method=sample num_chains=2";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError) << out.output;
}

TEST_F(CmdStan, multi_chain_multi_init_file_R) {
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " init=" << convert_model_path(init3_data)
     << " method=sample num_chains=4";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}
