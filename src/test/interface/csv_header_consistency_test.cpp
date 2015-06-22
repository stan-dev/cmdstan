#include <gtest/gtest.h>
#include <stan/services/error_codes.hpp>
#include <test/utility.hpp>
#include <stan/mcmc/chains.hpp>
#include <fstream>

using cmdstan::test::convert_model_path;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

TEST(interface,csv_header_consistency) {
  // from stan-dev/stan issue #109
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("csv_header_consistency");

  std::string path = convert_model_path(model_path);
  std::string samples = path + ".csv";

  std::string command
    = path
    + " sample num_warmup=0 num_samples=1"
    + " output file=" + samples;

  run_command_output out = run_command(command);
  EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code);
  EXPECT_FALSE(out.hasError);

  std::ifstream ifstream;
  ifstream.open(samples.c_str());
  stan::mcmc::chains<> chains(stan::io::stan_csv_reader::parse(ifstream, &std::cout));
  ifstream.close();
  
  EXPECT_EQ(1, chains.num_samples());
  EXPECT_FLOAT_EQ(1, chains.samples("z[1,1]")(0));
  EXPECT_FLOAT_EQ(2, chains.samples("z[1,2]")(0));
  EXPECT_FLOAT_EQ(3, chains.samples("z[2,1]")(0));
  EXPECT_FLOAT_EQ(4, chains.samples("z[2,2]")(0));
  EXPECT_FLOAT_EQ(1, chains.samples("z_mat[1,1]")(0));
  EXPECT_FLOAT_EQ(2, chains.samples("z_mat[1,2]")(0));
  EXPECT_FLOAT_EQ(3, chains.samples("z_mat[2,1]")(0));
  EXPECT_FLOAT_EQ(4, chains.samples("z_mat[2,2]")(0));
}
