#include <cmdstan/stansummary_helper.hpp>
#include <test/utility.hpp>
#include <stan/io/ends_with.hpp>
#include <fstream>
#include <sstream>
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
      = "      Mean  MCSE StdDev     MAD      10%       50%        90%"
        "    ESS_bulk     ESS_tail         R_hat\n";
  std::string expect_csv
      = "name,Mean,MCSE,StdDev,MAD,10%,50%,90%,ESS_bulk,ESS_tail,R_hat\n";
  std::vector<std::string> pcts;
  pcts.push_back("10");
  pcts.push_back("50");
  pcts.push_back("90");
  Eigen::VectorXd probs = percentiles_to_probs(pcts);
  EXPECT_FLOAT_EQ(probs[0], 0.1);
  EXPECT_FLOAT_EQ(probs[1], 0.5);
  EXPECT_FLOAT_EQ(probs[2], 0.9);

  std::vector<std::string> header = get_header(pcts);
  EXPECT_EQ(header.size(), pcts.size() + 7);
  EXPECT_EQ(header[0], "Mean");
  EXPECT_EQ(header[2], "StdDev");
  EXPECT_EQ(header[3], "MAD");
  EXPECT_EQ(header[5], "50%");
  EXPECT_EQ(header[7], "ESS_bulk");
  EXPECT_EQ(header[8], "ESS_tail");
  EXPECT_EQ(header[9], "R_hat");

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

TEST(CommandStansummary, percentiles) {
  std::vector<std::string> pcts;
  pcts.push_back("2.5");
  pcts.push_back("10");
  pcts.push_back("50");
  pcts.push_back("90");
  pcts.push_back("97.5");
  Eigen::VectorXd probs;
  EXPECT_NO_THROW(probs = percentiles_to_probs(pcts));
  EXPECT_FLOAT_EQ(probs[0], 0.025);
  EXPECT_FLOAT_EQ(probs[4], 0.975);

  pcts.clear();
  pcts.push_back("120");
  EXPECT_THROW(percentiles_to_probs(pcts), std::invalid_argument);

  pcts.clear();
  pcts.push_back("NAN");
  EXPECT_THROW(percentiles_to_probs(pcts), std::invalid_argument);

  pcts.clear();
  pcts.push_back("infinity");
  EXPECT_THROW(percentiles_to_probs(pcts), std::invalid_argument);

  pcts.clear();
  pcts.push_back("nonsenseString");
  EXPECT_THROW(percentiles_to_probs(pcts), std::invalid_argument);
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
  auto chains = parse_csv_files(filenames, metadata, warmup_times,
                                sampling_times, thin, &std::cout);
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

  Eigen::MatrixXd model_params(num_model_params, 10);
  std::vector<int> model_param_idxes(num_model_params);
  std::iota(model_param_idxes.begin(), model_param_idxes.end(),
            model_params_offset);
  get_stats(chains, sampling_times, probs, model_param_idxes, model_params);

  double mean_theta = model_params(0, 0);
  EXPECT_TRUE(mean_theta > 0.25);
  EXPECT_TRUE(mean_theta < 0.27);
  double rhat_theta = model_params(0, 9);
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

  arg_percentiles = "--percentiles \"101\"";
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

TEST(CommandStansummary, bad_include_param_args) {
  std::string expected_message
      = "--include_param: Unrecognized parameter(s): 'psi' ";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";
  std::string arg_include_param = "-i psi";

  run_command_output out
      = run_command(command + " " + arg_include_param + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";
}

TEST(CommandStansummary, check_console_output) {
  std::string lp
      = "lp__            -7.3  3.7e-02     0.77   0.30   -9.0  -7.0  -6.8      "
        " 519       503    1.0";
  std::string theta
      = "theta           0.26  6.1e-03     0.12   0.12  0.080  0.25  0.47      "
        " 362       396    1.0";
  std::string accept_stat
      = "accept_stat__   0.90  4.6e-03  1.5e-01  0.064   0.57  0.96   1.0      "
        "1284       941   1.00";
  std::string energy
      = "energy__         7.8  5.1e-02  1.0e+00   0.75    6.8   7.5   9.9      "
        " 490       486    1.0";

  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";

  run_command_output out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  std::istringstream target_stream(out.output);
  std::string line;

  // skip header
  std::getline(target_stream, line);  // model name
  std::getline(target_stream, line);  // chain info
  std::getline(target_stream, line);  // blank
  std::getline(target_stream, line);  // warmup time
  std::getline(target_stream, line);  // sample time
  std::getline(target_stream, line);  // blank
  std::getline(target_stream, line);  // header
  std::getline(target_stream, line);  // blank

  std::getline(target_stream, line);  // lp
  EXPECT_EQ(lp, line);
  std::getline(target_stream, line);  // accept stat
  EXPECT_EQ(accept_stat, line);
  std::getline(target_stream, line);  // stepsize
  std::getline(target_stream, line);  // treedepth
  std::getline(target_stream, line);  // n_leapfrog
  std::getline(target_stream, line);  // divergent

  std::getline(target_stream, line);  // energy
  EXPECT_EQ(energy, line);

  std::getline(target_stream, line);  // blank

  std::getline(target_stream, line);  // theta
  EXPECT_EQ(theta, line);
}

TEST(CommandStansummary, check_csv_output) {
  std::string csv_header
      = "name,Mean,MCSE,StdDev,MAD,5%,50%,95%,ESS_bulk,ESS_tail,R_hat";
  std::string lp
      = "\"lp__\",-7.2719,0.0365168,0.768874,0.303688,-8.98426,-6.97009,-6."
        "75007,519.29,503.309,1.00141";
  std::string energy
      = "\"energy__\",7.78428,0.0508815,1.0314,0.745859,6.80565,7.46758,9.8864,"
        "489.874,486.438,1.00495";
  std::string theta
      = "\"theta\",0.256552,0.00610844,0.119654,0.120965,0.0802982,0.24996,0."
        "47034,361.506,395.736,1.00186";

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

TEST(CommandStansummary, check_csv_output_no_percentiles) {
  std::string csv_header = "name,Mean,MCSE,StdDev,MAD,ESS_bulk,ESS_tail,R_hat";
  std::string lp
      = "\"lp__\",-7.2719,0.0365168,0.768874,0.303688,519.29,503.309,1.00141";

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

  std::string arg_percentiles = "-p \"\"";

  run_command_output out = run_command(command + " " + arg_csv_file + " "
                                       + csv_file + " " + arg_percentiles);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  std::ifstream target_stream(target_csv_file.c_str());
  if (!target_stream.is_open())
    FAIL();
  std::string line;
  std::getline(target_stream, line);
  EXPECT_EQ(csv_header, line);
  std::getline(target_stream, line);
  EXPECT_EQ(lp, line);
  target_stream.close();
  int return_code = std::remove(target_csv_file.c_str());
  if (return_code != 0)
    FAIL();
}

TEST(CommandStansummary, check_csv_output_sig_figs) {
  std::string csv_header
      = "name,Mean,MCSE,StdDev,MAD,5%,50%,95%,ESS_bulk,ESS_tail,R_hat";
  std::string lp = "\"lp__\",-7.3,0.037,0.77,0.3,-9,-7,-6.8,5.2e+02,5e+02,1";
  std::string energy
      = "\"energy__\",7.8,0.051,1,0.75,6.8,7.5,9.9,4.9e+02,4.9e+02,1";
  std::string theta
      = "\"theta\",0.26,0.0061,0.12,0.12,0.08,0.25,0.47,3.6e+02,4e+02,1";

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

TEST(CommandStansummary, check_csv_output_include_param) {
  std::string csv_header
      = "name,Mean,MCSE,StdDev,MAD,5%,50%,95%,ESS_bulk,ESS_tail,R_hat";
  std::string lp
      = "\"lp__\",-15.5617,0.97319,6.05585,6.3817,-25.3182,-15.7598,-5.47732,"
        "41.1897,113.537,1.00153";
  std::string energy
      = "\"energy__\",20.5888,1.01449,6.43127,6.6161,10.2809,20.8278,30.9921,"
        "42.5605,140.171,1.00069";
  // note: skipping theta 1-5
  std::string theta6
      = "\"theta[6]\",5.001,0.365016,5.76072,5.37947,-4.95375,5.22746,14.1688,"
        "230.645,464.978,1.00054";
  std::string theta7
      = "\"theta[7]\",8.54125,0.650098,6.22195,5.35785,-0.814388,8.09342,19."
        "2622,92.3075,241.177,1.00244";
  // note: skipping theta 8
  std::string message = "# Inference for Stan model: eight_schools_cp_model";

  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "eight_schools_output.csv";

  std::string target_csv_file = "src" + path_separator + "test" + path_separator
                                + "interface" + path_separator
                                + "example_output" + path_separator
                                + "tmp_test_target_csv_file.csv";
  std::string arg_csv_file = "--csv_filename=" + target_csv_file;

  // both styles of name supported
  std::string arg_include_param = "--include_param theta.6 -i theta[7]";

  run_command_output out = run_command(command + " " + arg_csv_file + " "
                                       + arg_include_param + " " + csv_file);

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
  EXPECT_EQ(theta6, line);
  std::getline(target_stream, line);
  EXPECT_EQ(theta7, line);
  std::getline(target_stream, line);
  EXPECT_EQ(message, line);
  target_stream.close();
  int return_code = std::remove(target_csv_file.c_str());
  if (return_code != 0)
    FAIL();
}
