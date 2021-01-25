#include <cmdstan/stansummary_helper.hpp>
#include <test/utility.hpp>
#include <stan/io/ends_with.hpp>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>
#include <CLI11/CLI11.hpp>

using cmdstan::test::count_matches;
using cmdstan::test::get_path_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

TEST(CommandStansummary, next_index_1d) {
  std::vector<int> dims(1);
  std::vector<int> index(1, 1);
  dims[0] = 100;

  ASSERT_EQ(1U, index.size());
  EXPECT_EQ(1, index[0]);
  for (int n = 1; n <= 100; n++) {
    if (n == 1)
      continue;
    next_index(index, dims);
    ASSERT_EQ(1U, index.size());
    EXPECT_EQ(n, index[0]);
  }

  index[0] = 100;
  EXPECT_THROW(next_index(index, dims), std::domain_error);

  index[0] = 1000;
  EXPECT_THROW(next_index(index, dims), std::domain_error);
}

TEST(CommandStansummary, next_index_2d) {
  std::vector<int> dims(2);
  std::vector<int> index(2, 1);
  dims[0] = 100;
  dims[1] = 3;

  ASSERT_EQ(2U, index.size());
  EXPECT_EQ(1, index[0]);
  EXPECT_EQ(1, index[1]);
  for (int i = 1; i <= 100; i++)
    for (int j = 1; j <= 3; j++) {
      if (i == 1 && j == 1)
        continue;
      next_index(index, dims);
      ASSERT_EQ(2U, index.size());
      EXPECT_EQ(i, index[0]);
      EXPECT_EQ(j, index[1]);
    }

  index[0] = 100;
  index[1] = 3;
  EXPECT_THROW(next_index(index, dims), std::domain_error);

  index[0] = 1000;
  index[1] = 1;
  EXPECT_THROW(next_index(index, dims), std::domain_error);

  index[0] = 10;
  index[1] = 4;
  EXPECT_NO_THROW(next_index(index, dims))
      << "this will correct the index and set the next element to (11,1)";
  EXPECT_EQ(11, index[0]);
  EXPECT_EQ(1, index[1]);
}

TEST(CommandStansummary, matrix_index_1d) {
  std::vector<int> dims(1);
  std::vector<int> index(1, 1);
  dims[0] = 100;

  EXPECT_EQ(0, matrix_index(index, dims));

  index[0] = 50;
  EXPECT_EQ(49, matrix_index(index, dims));

  index[0] = 100;
  EXPECT_EQ(99, matrix_index(index, dims));

  index[0] = 0;
  EXPECT_THROW(matrix_index(index, dims), std::domain_error);

  index[0] = 101;
  EXPECT_THROW(matrix_index(index, dims), std::domain_error);
}

TEST(CommandStansummary, matrix_index_2d) {
  std::vector<int> dims(2);
  std::vector<int> index(2, 1);
  dims[0] = 100;
  dims[1] = 3;

  EXPECT_EQ(0, matrix_index(index, dims));

  index[0] = 50;
  index[1] = 1;
  EXPECT_EQ(49, matrix_index(index, dims));

  index[0] = 100;
  index[1] = 1;
  EXPECT_EQ(99, matrix_index(index, dims));

  index[0] = 1;
  index[1] = 2;
  EXPECT_EQ(100, matrix_index(index, dims));

  index[0] = 1;
  index[1] = 3;
  EXPECT_EQ(200, matrix_index(index, dims));

  index[0] = 100;
  index[1] = 3;
  EXPECT_EQ(299, matrix_index(index, dims));

  index[0] = 1;
  index[1] = 0;
  EXPECT_THROW(matrix_index(index, dims), std::domain_error);

  index[0] = 0;
  index[1] = 1;
  EXPECT_THROW(matrix_index(index, dims), std::domain_error);

  index[0] = 101;
  index[1] = 1;
  EXPECT_THROW(matrix_index(index, dims), std::domain_error);

  index[0] = 1;
  index[1] = 4;
  EXPECT_THROW(matrix_index(index, dims), std::domain_error);
}

TEST(CommandStansummary, header_tests) {
  std::string expect
      = "      Mean  MCSE StdDev     10%      50%       90%      N_Eff     "
        "N_Eff/s        R_hat\n";
  std::string expect_csv
      = "name,Mean,MCSE,StdDev,10%,50%,90%,N_Eff,N_Eff/s,R_hat\n";
  std::vector<std::string> pcts;
  pcts.push_back("10");
  pcts.push_back("50");
  pcts.push_back("90");
  Eigen::VectorXd probs = percentiles_to_probs(pcts);
  EXPECT_FLOAT_EQ(probs[0], 0.1);
  EXPECT_FLOAT_EQ(probs[1], 0.5);
  EXPECT_FLOAT_EQ(probs[2], 0.9);

  std::vector<std::string> header = get_header(pcts);
  EXPECT_EQ(header.size(), pcts.size() + 6);
  EXPECT_EQ(header[0], "Mean");
  EXPECT_EQ(header[2], "StdDev");
  EXPECT_EQ(header[4], "50%");
  EXPECT_EQ(header[6], "N_Eff");
  EXPECT_EQ(header[8], "R_hat");

  Eigen::VectorXi column_widths(header.size());
  for (size_t i = 0, w = 5; i < header.size(); ++i, ++w) {
    column_widths[i] = w;
  }
  std::stringstream ss;
  write_header(header, column_widths, 4, false, &ss);
  EXPECT_EQ(expect, ss.str());
  ss.str(std::string());
  write_header(header, column_widths, 24, true, &ss);
  EXPECT_EQ(expect_csv, ss.str());
}

TEST(CommandStansummary, param_tests) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());

  std::vector<std::string> pcts;
  pcts.push_back("10");
  pcts.push_back("50");
  pcts.push_back("90");
  Eigen::VectorXd probs = percentiles_to_probs(pcts);

  // bernoulli model:  6 sampler params, ("lp__", is model param)
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";
  std::vector<std::string> filenames;
  filenames.push_back(csv_file);
  stan::io::stan_csv_metadata metadata;
  Eigen::VectorXd warmup_times(filenames.size());
  Eigen::VectorXd sampling_times(filenames.size());
  Eigen::VectorXi thin(filenames.size());
  stan::mcmc::chains<> chains = parse_csv_files(
      filenames, metadata, warmup_times, sampling_times, thin, &std::cout);
  EXPECT_EQ(chains.num_chains(), 1);
  EXPECT_EQ(chains.num_params(), 8);

  size_t max_name_length = 0;
  size_t num_sampler_params = -1;  // don't count name 'lp__'
  for (int i = 0; i < chains.num_params(); ++i) {
    if (chains.param_name(i).length() > max_name_length)
      max_name_length = chains.param_name(i).length();
    if (stan::io::ends_with("__", chains.param_name(i)))
      num_sampler_params++;
  }
  EXPECT_EQ(num_sampler_params, 6);

  size_t model_params_offset = num_sampler_params + 1;
  size_t num_model_params = chains.num_params() - model_params_offset;
  EXPECT_EQ(num_model_params, 1);

  Eigen::MatrixXd model_params(num_model_params, 9);
  get_stats(chains, warmup_times, sampling_times, probs, model_params_offset,
            model_params);

  double mean_theta = model_params(0, 0);
  EXPECT_TRUE(mean_theta > 0.25);
  EXPECT_TRUE(mean_theta < 0.27);
  double rhat_theta = model_params(0, 8);
  EXPECT_TRUE(rhat_theta > 0.999);
  EXPECT_TRUE(rhat_theta < 1.01);
}

// good csv file, no draws
TEST(CommandStansummary, functional_test__issue_342) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "matrix_output.csv";
  run_command_output out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";
}

// good csv file, variational inference
TEST(CommandStansummary, variational_inference) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator
                         + "variational_inference_output.csv";
  run_command_output out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";
}

TEST(CommandStansummary, no_args) {
  std::string expected_message = "Usage: stansummary";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  run_command_output out = run_command(command);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";
}

TEST(CommandStansummary, bad_input_files) {
  std::string expected_message = "File does not exist";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "no_such_file";
  run_command_output out = run_command(command + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";
}

TEST(CommandStansummary, bad_csv_file_arg) {
  std::string expected_message = "Cannot save to csv_filename";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";
  std::string arg_csv_file = "--csv_filename " + path_separator + "bin"
                             + path_separator + "hi_mom.csv";

  run_command_output out
      = run_command(command + " " + arg_csv_file + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";
}

TEST(CommandStansummary, bad_sig_figs_arg) {
  std::string expected_message = "--sig_figs: Value -1 not in range";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";
  std::string arg_sig_figs = "--sig_figs -1";

  run_command_output out
      = run_command(command + " " + arg_sig_figs + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";

  arg_sig_figs = "--sig_figs 101";
  expected_message = "--sig_figs: Value 101 not in range";
  out = run_command(command + " " + arg_sig_figs + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";
}

TEST(CommandStansummary, bad_autocorr_arg) {
  std::string expected_message = "--autocorr: Number less or equal to 0";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";
  std::string arg_autocorr = "--autocorr -1";

  run_command_output out
      = run_command(command + " " + arg_autocorr + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";

  arg_autocorr = "--autocorr 0";
  out = run_command(command + " " + arg_autocorr + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";

  expected_message = "not a valid chain id";
  arg_autocorr = "--autocorr 2";
  out = run_command(command + " " + arg_autocorr + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";
}

TEST(CommandStansummary, bad_percentiles_arg) {
  std::string expected_message = "Option --percentiles";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";

  std::string arg_percentiles = "--percentiles -1";
  run_command_output out
      = run_command(command + " " + arg_percentiles + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";

  arg_percentiles = "--percentiles \"0,100\"";
  out = run_command(command + " " + arg_percentiles + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";

  arg_percentiles = "--percentiles \"2,30,5\"";
  out = run_command(command + " " + arg_percentiles + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";

  arg_percentiles = "--percentiles \"2,50,95\"";
  out = run_command(command + " " + arg_percentiles + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";
}

TEST(CommandStansummary, check_console_output) {
  std::string lp
      = "lp__            -7.3  3.7e-02     0.77   -9.1  -7.0  -6.8      443    "
        "19275      1.0";
  std::string theta
      = "theta           0.26  6.1e-03     0.12  0.079  0.25  0.47      384    "
        "16683     1.00";
  std::string accept_stat
      = "accept_stat__   0.90  4.6e-03  1.5e-01   0.57  0.96   1.0  1.0e+03  "
        "4.5e+04  1.0e+00";
  std::string energy
      = "energy__         7.8  5.1e-02  1.0e+00    6.8   7.5   9.9  4.1e+02  "
        "1.8e+04  1.0e+00";

  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";

  run_command_output out = run_command(command + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, lp));
  EXPECT_TRUE(boost::algorithm::contains(out.output, theta));
  EXPECT_TRUE(boost::algorithm::contains(out.output, accept_stat));
  EXPECT_TRUE(boost::algorithm::contains(out.output, energy));
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";
}

TEST(CommandStansummary, check_csv_output) {
  std::string csv_header
      = "name,Mean,MCSE,StdDev,5%,50%,95%,N_Eff,N_Eff/s,R_hat";
  std::string lp
      = "\"lp__\",-7.2719,0.0365168,0.768874,-9.05757,-6.96978,-6.75008,443."
        "328,19275.1,1.00037";
  std::string energy
      = "\"energy__\""
        ",7.78428,0.0508815,1.0314,6.80383,7.46839,9.88601,410."
        "898,17865.1,1.00075";
  std::string theta
      = "\"theta\",0.256552,0.00610844,0.119654,0.0786292,0.24996,0.470263,383."
        "704,16682.8,0.999309";

  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";

  std::string target_csv_file = "src" + path_separator + "test" + path_separator
                                + "interface" + path_separator
                                + "example_output" + path_separator
                                + "tmp_test_target_csv_file.csv";
  std::string arg_csv_file = "--csv_filename=" + target_csv_file;

  run_command_output out
      = run_command(command + " " + arg_csv_file + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  std::ifstream target_stream(target_csv_file.c_str());
  if (!target_stream.is_open())
    FAIL();
  std::string line;
  std::getline(target_stream, line);
  EXPECT_EQ(csv_header, line);
  std::getline(target_stream, line);
  EXPECT_EQ(lp, line);
  std::getline(target_stream, line);  // accept_stat
  std::getline(target_stream, line);  // stepsize
  std::getline(target_stream, line);  // treedepth
  std::getline(target_stream, line);  // n_leapfrog
  std::getline(target_stream, line);  // divergent
  std::getline(target_stream, line);  // energy
  EXPECT_EQ(energy, line);
  std::getline(target_stream, line);
  EXPECT_EQ(theta, line);
  target_stream.close();
  int return_code = std::remove(target_csv_file.c_str());
  if (return_code != 0)
    FAIL();
}

TEST(CommandStansummary, check_csv_output_sig_figs) {
  std::string csv_header
      = "name,Mean,MCSE,StdDev,5%,50%,95%,N_Eff,N_Eff/s,R_hat";
  std::string lp = "\"lp__\",-7.3,0.037,0.77,-9.1,-7,-6.8,4.4e+02,1.9e+04,1";
  std::string energy = "\"energy__\",7.8,0.051,1,6.8,7.5,9.9,4.1e+02,1.8e+04,1";
  std::string theta
      = "\"theta\",0.26,0.0061,0.12,0.079,0.25,0.47,3.8e+02,1.7e+04,1";

  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";

  std::string target_csv_file = "src" + path_separator + "test" + path_separator
                                + "interface" + path_separator
                                + "example_output" + path_separator
                                + "tmp_test_target_csv_file.csv";
  std::string arg_csv_file = "--csv_filename " + target_csv_file;
  std::string arg_sig_figs = "--sig_figs 2";

  run_command_output out = run_command(command + " " + arg_csv_file + " "
                                       + arg_sig_figs + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  std::ifstream target_stream(target_csv_file.c_str());
  if (!target_stream.is_open())
    FAIL();
  std::string line;
  std::getline(target_stream, line);
  EXPECT_EQ(csv_header, line);
  std::getline(target_stream, line);
  EXPECT_EQ(lp, line);
  std::getline(target_stream, line);  // accept_stat
  std::getline(target_stream, line);  // stepsize
  std::getline(target_stream, line);  // treedepth
  std::getline(target_stream, line);  // n_leapfrog
  std::getline(target_stream, line);  // divergent
  std::getline(target_stream, line);  // energy
  EXPECT_EQ(energy, line);
  std::getline(target_stream, line);
  EXPECT_EQ(theta, line);
  target_stream.close();
  int return_code = std::remove(target_csv_file.c_str());
  if (return_code != 0)
    FAIL();
}
