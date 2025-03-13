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
    init2_3_data = {"src", "test", "test-models", "bern_init2_3.json"};
    init3_data = {"src", "test", "test-models", "bern_init2.R"};
    init_bad_data = {"src", "test", "test-models", "bern_init_bad.json"};
    init_bad_2_data = {"src", "test", "test-models", "bern_init_bad_2.json"};
    dummy_output = {"test", "ignored.csv"};
  }
  std::vector<std::string> bern_model;
  std::vector<std::string> dummy_output;
  std::vector<std::string> bern_data;
  std::vector<std::string> init_data;
  std::vector<std::string> init2_data;
  std::vector<std::string> init2_3_data;
  std::vector<std::string> init3_data;
  std::vector<std::string> init_bad_data;
  std::vector<std::string> init_bad_2_data;
};

TEST_F(CmdStan, multi_chain_single_init_file_good) {
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dummy_output)
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
     << " output file=" << convert_model_path(dummy_output)
     << " init=" << convert_model_path(init2_data)
     << " method=sample num_chains=4";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, multi_chain_multi_init_file_comma_good) {
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dummy_output)
     << " init=" << convert_model_path(init_data) << ","
     << convert_model_path(init2_3_data) << " method=sample num_chains=2";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError) << out.output;
}

TEST_F(CmdStan, multi_chain_multi_init_file_id_good) {
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dummy_output)
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
     << " output file=" << convert_model_path(dummy_output)
     << " init=" << convert_model_path(init2_data) << " id=4"
     << " method=sample num_chains=3";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  EXPECT_TRUE(out.hasError);
  EXPECT_IN_STRING("Cannot open some of the requested files", out.output);
  EXPECT_IN_STRING("In this case, neither option was found.", out.output);
}

TEST_F(CmdStan, multi_chain_multi_init_file_comma_missing) {
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data) << " output file="
     << convert_model_path(dummy_output)
     //  second init file does not exist
     << " init=" << convert_model_path(init2_data) << ","
     << convert_model_path(init2_data) << " method=sample num_chains=2";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  EXPECT_TRUE(out.hasError);
  EXPECT_IN_STRING("Cannot open some of the requested files", out.output);
}

TEST_F(CmdStan, multi_chain_multi_init_file_comma_wrong_number) {
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dummy_output)
     << " init=" << convert_model_path(init_data) << ","
     << convert_model_path(init2_3_data) << " method=sample num_chains=3";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  EXPECT_TRUE(out.hasError) << out.output;
  EXPECT_IN_STRING("Number of filenames does not match number of chains",
                   out.output);
}

TEST_F(CmdStan, multi_chain_multi_init_file_actually_used) {
  // the second chain has a bad init value
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dummy_output)
     << " init=" << convert_model_path(init_bad_data)
     << " method=sample num_chains=2";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  EXPECT_TRUE(out.hasError) << out.output;
  EXPECT_IN_STRING("User-specified initialization failed.", out.output);
}

TEST_F(CmdStan, multi_chain_multi_init_file_actually_used_comma) {
  // the second chain has a bad init value
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dummy_output)
     << " init=" << convert_model_path(init_data) << ","
     << convert_model_path(init_bad_2_data) << " method=sample num_chains=2";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  EXPECT_TRUE(out.hasError) << out.output;
  EXPECT_IN_STRING("User-specified initialization failed.", out.output);
}

TEST_F(CmdStan, multi_chain_multi_init_file_R) {
  std::stringstream ss;
  ss << convert_model_path(bern_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dummy_output)
     << " init=" << convert_model_path(init3_data)
     << " method=sample num_chains=4";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  EXPECT_FALSE(out.hasError);
  EXPECT_IN_STRING(
      "This format is deprecated and will not receive new features",
      out.output);
}
