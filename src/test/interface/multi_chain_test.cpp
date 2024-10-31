#include <cmdstan/stansummary_helper.hpp>
#include <stan/io/stan_csv_reader.hpp>
#include <stan/services/error_codes.hpp>
#include <stan/mcmc/chainset.hpp>
#include <test/utility.hpp>
#include <gtest/gtest.h>

TEST(interface, output_multi) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("test_model");

  std::string command
      = cmdstan::test::convert_model_path(model_path)
        + " id=10 sample num_warmup=200 num_samples=1 num_chains=2 random seed=1234"
        + " output file=" + cmdstan::test::convert_model_path(model_path)
        + ".csv diagnostic_file=" + cmdstan::test::convert_model_path(model_path) + "_diag.csv";

  cmdstan::test::run_command_output out = cmdstan::test::run_command(command);
  EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code);
  EXPECT_FALSE(out.hasError);
  {
    std::string csv_file
        = cmdstan::test::convert_model_path(model_path) + "_10.csv";
    std::ifstream infile;
    std::stringstream warnings;
    stan::io::stan_csv sample;
    infile.open(csv_file.c_str());
    sample = stan::io::stan_csv_reader::parse(infile, &warnings);
    stan::mcmc::chainset chains(sample);
    constexpr std::array<const char*, 9> names{
        "lp__",        "accept_stat__", "stepsize__",
        "treedepth__", "n_leapfrog__",  "divergent__",
        "energy__",    "mu1",           "mu2",
    };
    const auto chain_param_names = chains.param_names();
    for (size_t i = 0; i < 9; ++i) {
      EXPECT_EQ(names[i], chain_param_names[i]);
    }
    std::string diag_name
        = cmdstan::test::convert_model_path(model_path) + "_diag_10.csv";
    std::ifstream diag_file(diag_name);
    EXPECT_TRUE(diag_file.good());
  }
  {
    std::string csv_file
        = cmdstan::test::convert_model_path(model_path) + "_11.csv";
    std::ifstream infile;
    std::stringstream warnings;
    stan::io::stan_csv sample;
    infile.open(csv_file.c_str());
    sample = stan::io::stan_csv_reader::parse(infile, &warnings);
    stan::mcmc::chainset chains(sample);
    constexpr std::array<const char*, 9> names{
        "lp__",        "accept_stat__", "stepsize__",
        "treedepth__", "n_leapfrog__",  "divergent__",
        "energy__",    "mu1",           "mu2",
    };
    const auto chain_param_names = chains.param_names();
    for (size_t i = 0; i < 9; ++i) {
      EXPECT_EQ(names[i], chain_param_names[i]);
    }
    std::string diag_name
        = cmdstan::test::convert_model_path(model_path) + "_diag_11.csv";
    std::ifstream diag_file(diag_name);
    EXPECT_TRUE(diag_file.good());
  }
}
