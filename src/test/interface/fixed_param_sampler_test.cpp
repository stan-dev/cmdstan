#include <test/utility.hpp>
#include <stan/io/stan_csv_reader.hpp>
#include <stan/mcmc/chains.hpp>
#include <stan/mcmc/fixed_param_sampler.hpp>
#include <gtest/gtest.h>
#include <fstream>
#include <boost/algorithm/string.hpp>

using cmdstan::test::convert_model_path;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

void test_constant(const Eigen::VectorXd &samples) {
  for (int i = 1; i < samples.size(); i++)
    EXPECT_EQ(samples(0), samples(i));
}

TEST(McmcPersistentSampler, check_persistency) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("proper");

  std::string command = convert_model_path(model_path);
  command += " sample algorithm=fixed_param output file="
             + convert_model_path(model_path) + ".csv";
  run_command_output command_output;

  try {
    command_output = run_command(command);
  } catch (...) {
    ADD_FAILURE() << "Failed running command: " << command;
  }

  std::ifstream output_stream;
  output_stream.open((convert_model_path(model_path) + ".csv").data());

  stan::io::stan_csv parsed_output
      = stan::io::stan_csv_reader::parse(output_stream, 0);
  stan::mcmc::chains<> chains(parsed_output);

  for (int i = 0; i < chains.num_params(); ++i) {
    test_constant(chains.samples(0, i));
  }
}

TEST(McmcFixedParamSampler, check_empty) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("empty");

  std::string command = convert_model_path(model_path);
  command += " sample algorithm=fixed_param output file="
             + convert_model_path(model_path) + ".csv";
  run_command_output command_output;

  bool success = true;

  try {
    command_output = run_command(command);
  } catch (...) {
    success = false;
  }

  EXPECT_EQ(success, true);
}

TEST(McmcFixedParamSampler, check_empty_but_algorithm_not_fixed_param) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("empty");

  std::string command = convert_model_path(model_path);
  command += " sample output file=" + convert_model_path(model_path) + ".csv";
  run_command_output command_output;

  bool success = true;

  try {
    command_output = run_command(command);
  } catch (...) {
    success = false;
  }

  EXPECT_EQ(success, true);
  std::string expected_message
      = "Model contains no parameters, running fixed_param sampler";
  EXPECT_TRUE(
      boost::algorithm::contains(command_output.output, expected_message));
}
