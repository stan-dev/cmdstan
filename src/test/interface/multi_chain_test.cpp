#include <cmdstan/stansummary_helper.hpp>
#include <stan/io/stan_csv_reader.hpp>
#include <stan/services/error_codes.hpp>
#include <stan/mcmc/chains.hpp>
#include <test/utility.hpp>
#include <gtest/gtest.h>

TEST(interface, output_multi) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("proper_sig_figs");

  std::string command
      = cmdstan::test::convert_model_path(model_path)
        + " sample num_warmup=200 num_samples=1 num_chains=2" + " output file="
        + cmdstan::test::convert_model_path(model_path) + ".csv";

  cmdstan::test::run_command_output out = cmdstan::test::run_command(command);
  EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code);
  EXPECT_FALSE(out.hasError);
  {
  std::string csv_file = cmdstan::test::convert_model_path(model_path) + "_1.csv";
  std::vector<std::string> filenames;
  filenames.push_back(csv_file);
  stan::io::stan_csv_metadata metadata;
  Eigen::VectorXd warmup_times(filenames.size());
  Eigen::VectorXd sampling_times(filenames.size());
  Eigen::VectorXi thin(filenames.size());
  stan::mcmc::chains<> chains = parse_csv_files(
      filenames, metadata, warmup_times, sampling_times, thin, &std::cout);
  EXPECT_NEAR(chains.samples(8)(0, 0), 0.1, 1E-16);
}
{
  std::string csv_file = cmdstan::test::convert_model_path(model_path) + "_2.csv";
  std::vector<std::string> filenames;
  filenames.push_back(csv_file);
  stan::io::stan_csv_metadata metadata;
  Eigen::VectorXd warmup_times(filenames.size());
  Eigen::VectorXd sampling_times(filenames.size());
  Eigen::VectorXi thin(filenames.size());
  stan::mcmc::chains<> chains = parse_csv_files(
      filenames, metadata, warmup_times, sampling_times, thin, &std::cout);
  EXPECT_NEAR(chains.samples(8)(0, 0), 0.1, 1E-16);
}
}
