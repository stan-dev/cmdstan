#include <cmdstan/stansummary_helper.hpp>
#include <test/utility.hpp>
#include <stan/io/ends_with.hpp>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>
#include <CLI11/CLI11.hpp>
#include <boost/tokenizer.hpp>

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

TEST(CommandStansummary, order_param_names_row_major) {
  std::vector<std::string> names_col_major = {
    "lp__", "foo", "bar", "theta[1]", "theta[2]", "theta[3]",
    "gamma[1,1]", "gamma[2,1]", "gamma[3,1]",
    "gamma[1,2]", "gamma[2,2]", "gamma[3,2]",
    "rho[1,1,1]", "rho[2,1,1]", "rho[3,1,1]",
    "rho[1,2,1]", "rho[2,2,1]", "rho[3,2,1]",
    "rho[1,1,2]", "rho[2,1,2]", "rho[3,1,2]",
    "rho[1,2,2]", "rho[2,2,2]", "rho[3,2,2]",
    "zeta" };
  std::vector<std::string> expect_row_major = {
    "lp__", "foo", "bar", "theta[1]", "theta[2]", "theta[3]",
    "gamma[1,1]", "gamma[1,2]", "gamma[2,1]",
    "gamma[2,2]", "gamma[3,1]", "gamma[3,2]",
    "rho[1,1,1]", "rho[1,1,2]", "rho[1,2,1]",
    "rho[1,2,2]", "rho[2,1,1]", "rho[2,1,2]",
    "rho[2,2,1]", "rho[2,2,2]", "rho[3,1,1]",
    "rho[3,1,2]", "rho[3,2,1]", "rho[3,2,2]",
    "zeta" };
  auto names_row_major = order_param_names_row_major(names_col_major);
  for (size_t i = 0; i < names_col_major.size(); ++i) {
    EXPECT_EQ(expect_row_major[i], names_row_major[i]);
  }
}

TEST(CommandStansummary, order_param_names_requested) {
  std::vector<std::string> requested_params = {
    "theta[2]", "theta[3]",
    "rho[1,1,1]", "rho[1,1,2]", "rho[1,2,1]",
    "rho[1,2,2]", "rho[1,3,1]", "rho[1,3,2]",
    "zeta" };
  auto names_row_major = order_param_names_row_major(requested_params);
   for (size_t i = 0; i < requested_params.size(); ++i) {
    EXPECT_EQ(requested_params[i], names_row_major[i]);
  }
}

TEST(CommandStansummary, header_tests) {
  std::string expect_console
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
  EXPECT_EQ(header[1], "MCSE");
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
  EXPECT_EQ(expect_console, ss.str());
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

// good csv file, no draws
TEST(CommandStansummary, functional_test__issue_342) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "matrix_output.csv";
  run_command_output out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError);
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
  ASSERT_FALSE(out.hasError);
}

TEST(CommandStansummary, no_args) {
  std::string expected_message = "Usage: stansummary";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  run_command_output out = run_command(command);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError);
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
  ASSERT_TRUE(out.hasError);
}

TEST(CommandStansummary, bad_csv_file_arg) {
  std::string expected_message = "Cannot save to csv_filename";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bern1.csv";
  std::string arg_csv_file = "--csv_filename " + path_separator + "bin"
                             + path_separator + "hi_mom.csv";

  run_command_output out
      = run_command(command + " " + arg_csv_file + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError);
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
  ASSERT_TRUE(out.hasError);

  arg_sig_figs = "--sig_figs 101";
  expected_message = "--sig_figs: Value 101 not in range";
  out = run_command(command + " " + arg_sig_figs + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError);
}

TEST(CommandStansummary, bad_autocorr_arg) {
  std::string expected_message = "--autocorr: Number less or equal to 0";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bern1.csv";
  std::string arg_autocorr = "--autocorr -1";

  run_command_output out
      = run_command(command + " " + arg_autocorr + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError);

  arg_autocorr = "--autocorr 0";
  out = run_command(command + " " + arg_autocorr + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError);

  expected_message = "not a valid chain id";
  arg_autocorr = "--autocorr 2";
  out = run_command(command + " " + arg_autocorr + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError);
}

TEST(CommandStansummary, bad_percentiles_arg) {
  std::string expected_message = "Option --percentiles";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bern1.csv";

  std::string arg_percentiles = "--percentiles -1";
  run_command_output out
      = run_command(command + " " + arg_percentiles + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError);

  arg_percentiles = "--percentiles \"0,100.001\"";
  out = run_command(command + " " + arg_percentiles + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError);

  arg_percentiles = "--percentiles \"2,30,5\"";
  out = run_command(command + " " + arg_percentiles + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError);

  arg_percentiles = "--percentiles \"2,50,95\"";
  out = run_command(command + " " + arg_percentiles + " " + csv_file);
  ASSERT_FALSE(out.hasError);
}

TEST(CommandStansummary, bad_include_param_args) {
  std::string expected_message
      = "--include_param: Unrecognized parameter(s): 'psi' ";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bern1.csv";
  std::string arg_include_param = "-i psi";

  run_command_output out
      = run_command(command + " " + arg_include_param + " -f " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError);
}

TEST(CommandStansummary, input_files_flag) {
  std::string expected_message
      = "--include_param: Unrecognized parameter(s): 'psi' ";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string arg_csv_files = "-f src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bern1.csv";
  std::string arg_percentiles = "--percentiles \"2,50,95\"";
  std::string arg_include_param = "-i theta divergent__";

  run_command_output out
      = run_command(command + " " + arg_csv_files + " " + arg_percentiles
		    + " " + arg_include_param);
  ASSERT_FALSE(out.hasError);

  out = run_command(command + " " + arg_percentiles + " " + arg_csv_files
		    + " " + arg_include_param);
  ASSERT_FALSE(out.hasError);

  out = run_command(command + " " + arg_percentiles + " " + arg_include_param
		    + " " + arg_csv_files);
  ASSERT_FALSE(out.hasError);
}

TEST(CommandStansummary, check_console_output) {
  std::vector<std::string> header
      = {"Mean", "MCSE", "StdDev",   "MAD",      "5%",
         "50%",  "95%",  "ESS_bulk", "ESS_tail", "R_hat"};
  std::vector<std::string> divergent
      = {"divergent__", "0.00", "nan", "0.00", "0.00", "0.00", "0.00", "0.00"};
  std::vector<std::string> theta
      = {"theta", "0.25", "0.0033", "0.12", "0.12", "0.077",
         "0.24",  "0.47", "1408",   "1292", "1.0"};

  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string csv_dir = "src" + path_separator + "test" + path_separator
                        + "interface" + path_separator + "example_output"
                        + path_separator;
  std::stringstream ss_command;
  ss_command << "bin" << path_separator << "stansummary ";
  for (int i = 1; i < 5; ++i) {
    ss_command << " " << csv_dir << "bern" << i << ".csv";
  }
  run_command_output out = run_command(ss_command.str());
  ASSERT_FALSE(out.hasError);
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
  boost::char_separator<char> sep(" \t\n");
  boost::tokenizer<boost::char_separator<char>> header_tok(line, sep);
  int i = 0;
  for (const auto& token : header_tok) {
    EXPECT_EQ(token, header[i]);
    i++;
  }
  std::getline(target_stream, line);  // blank
  std::getline(target_stream, line);  // lp
  std::getline(target_stream, line);  // accept stat
  std::getline(target_stream, line);  // stepsize
  std::getline(target_stream, line);  // treedepth
  std::getline(target_stream, line);  // n_leapfrog
  std::getline(target_stream, line);  // divergent
  boost::tokenizer<boost::char_separator<char>> divergent_tok(line, sep);
  i = 0;
  for (const auto& token : divergent_tok) {
    EXPECT_EQ(token, divergent[i]);
    i++;
  }
  std::getline(target_stream, line);  // energy
  std::getline(target_stream, line);  // blank
  std::getline(target_stream, line);  // theta
  boost::tokenizer<boost::char_separator<char>> theta_tok(line, sep);
  i = 0;
  for (const auto& token : theta_tok) {
    EXPECT_EQ(token, theta[i]);
    i++;
  }
}

TEST(CommandStansummary, check_csv_output) {
  std::string csv_header
      = "name,Mean,MCSE,StdDev,MAD,5%,50%,95%,ESS_bulk,ESS_tail,R_hat";
  std::string lp
      = "\"lp__\",-7.28676,0.0202121,0.738616,0.329575,-8.7823,-6.99022,-6."
        "7503,1512.77,1591.97,1.00073";
  std::string theta
      = "\"theta\",0.251297,0.00327489,0.121547,0.123091,0.0771694,0.237476,0."
        "473885,1407.51,1291.71,1.00679";

  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string csv_dir = "src" + path_separator + "test" + path_separator
                        + "interface" + path_separator + "example_output"
                        + path_separator;
  std::string target_csv_file = "test" + path_separator + "interface"
                                + path_separator
                                + "tmp_test_target_csv_file_a.csv";

  std::stringstream ss_command;
  ss_command << "bin" << path_separator << "stansummary "
             << "--csv_filename=" << target_csv_file;
  for (int i = 1; i < 5; ++i) {
    ss_command << " " << csv_dir << "bern" << i << ".csv";
  }
  run_command_output out = run_command(ss_command.str());
  ASSERT_FALSE(out.hasError);

  std::ifstream target_stream(target_csv_file.c_str());
  if (!target_stream.is_open()) {
    std::cerr << "Failed to open file: " << target_csv_file << "\n";
    std::cerr << "Error: " << std::strerror(errno) << std::endl;
    FAIL();
  }
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
  std::getline(target_stream, line);
  EXPECT_EQ(theta, line);
  target_stream.close();
  int return_code = std::remove(target_csv_file.c_str());
  if (return_code != 0) {
    std::cerr << "Failed to remove file: " << target_csv_file << "\n";
    std::cerr << "Error: " << std::strerror(errno) << std::endl;
    FAIL();
  }
}

TEST(CommandStansummary, check_csv_output_no_percentiles) {
  std::string csv_header = "name,Mean,MCSE,StdDev,MAD,ESS_bulk,ESS_tail,R_hat";
  std::string lp
      = "\"lp__\",-7.28676,0.0202121,0.738616,0.329575,1512.77,1591.97,1.00073";

  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string csv_dir = "src" + path_separator + "test" + path_separator
                        + "interface" + path_separator + "example_output"
                        + path_separator;
  std::string target_csv_file = "test" + path_separator + "interface"
                                + path_separator
                                + "tmp_test_target_csv_file_b.csv";
  std::string arg_percentiles = " -p \"\"";
  std::stringstream ss_command;
  ss_command << "bin" << path_separator << "stansummary "
             << "--csv_filename=" << target_csv_file << arg_percentiles;
  for (int i = 1; i < 5; ++i) {
    ss_command << " " << csv_dir << "bern" << i << ".csv";
  }
  run_command_output out = run_command(ss_command.str());
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  std::ifstream target_stream(target_csv_file.c_str());
  if (!target_stream.is_open()) {
    std::cerr << "Failed to open file: " << target_csv_file << "\n";
    std::cerr << "Error: " << std::strerror(errno) << std::endl;
    FAIL();
  }
  std::string line;
  std::getline(target_stream, line);
  EXPECT_EQ(csv_header, line);
  std::getline(target_stream, line);
  EXPECT_EQ(lp, line);
  target_stream.close();
  int return_code = std::remove(target_csv_file.c_str());
  if (return_code != 0) {
    std::cerr << "Failed to remove file: " << target_csv_file << "\n";
    std::cerr << "Error: " << std::strerror(errno) << std::endl;
    FAIL();
  }
}

TEST(CommandStansummary, check_csv_output_sig_figs) {
  std::string csv_header
      = "name,Mean,MCSE,StdDev,MAD,5%,50%,95%,ESS_bulk,ESS_tail,R_hat";
  std::string lp
      = "\"lp__\",-7.3,0.02,0.74,0.33,-8.8,-7,-6.8,1.5e+03,1.6e+03,1";
  std::string energy
      = "\"energy__\",7.8,0.027,1,0.74,6.8,7.5,9.7,1.4e+03,1.8e+03,1";
  std::string theta
      = "\"theta\",0.25,0.0033,0.12,0.12,0.077,0.24,0.47,1.4e+03,1.3e+03,1";

  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string csv_dir = "src" + path_separator + "test" + path_separator
                        + "interface" + path_separator + "example_output"
                        + path_separator;
  std::string target_csv_file = "test" + path_separator + "interface"
                                + path_separator
                                + "tmp_test_target_csv_file_c.csv";

  std::stringstream ss_command;
  ss_command << "bin" << path_separator << "stansummary "
             << "--csv_filename=" << target_csv_file << " --sig_figs 2";
  for (int i = 1; i < 5; ++i) {
    ss_command << " " << csv_dir << "bern" << i << ".csv";
  }
  run_command_output out = run_command(ss_command.str());
  ASSERT_FALSE(out.hasError);

  std::ifstream target_stream(target_csv_file.c_str());
  if (!target_stream.is_open()) {
    std::cerr << "Failed to open file: " << target_csv_file << "\n";
    std::cerr << "Error: " << std::strerror(errno) << std::endl;
    FAIL();
  }
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
  if (return_code != 0) {
    std::cerr << "Failed to remove file: " << target_csv_file << "\n";
    std::cerr << "Error: " << std::strerror(errno) << std::endl;
    FAIL();
  }
}

TEST(CommandStansummary, check_csv_output_include_param) {
  std::string csv_header
      = "name,Mean,MCSE,StdDev,MAD,5%,50%,95%,ESS_bulk,ESS_tail,R_hat";
  // note: skipping theta 1-5
  std::string theta6
      = "\"theta[6]\",5.001,0.365016,5.76072,5.37947,-4.95375,5.22746,14.1688,"
        "230.645,464.978,1.00054";
  std::string theta7
      = "\"theta[7]\",8.54125,0.650098,6.22195,5.35785,-0.814388,8.09342,19."
        "2622,92.3075,241.177,1.00244";
  std::string message = "# Inference for Stan model: eight_schools_cp_model";

  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string csv_dir = "src" + path_separator + "test" + path_separator
                        + "interface" + path_separator + "example_output"
                        + path_separator;
  std::string target_csv_file = "test" + path_separator + "interface"
                                + path_separator
                                + "tmp_test_target_csv_file_d.csv";

  std::stringstream ss_command;
  ss_command << "bin" << path_separator << "stansummary "
             << "--csv_filename=" << target_csv_file
             << " --include_param theta.6 -i theta[7] -f " << csv_dir
             << "eight_schools_output.csv";
  run_command_output out = run_command(ss_command.str());
  ASSERT_FALSE(out.hasError);

  std::ifstream target_stream(target_csv_file.c_str());
  if (!target_stream.is_open()) {
    std::cerr << "Failed to open file: " << target_csv_file << "\n";
    std::cerr << "Error: " << std::strerror(errno) << std::endl;
    FAIL();
  }
  std::string line;
  std::getline(target_stream, line);
  EXPECT_EQ(csv_header, line);
  std::getline(target_stream, line);
  EXPECT_EQ(theta6, line);
  std::getline(target_stream, line);
  EXPECT_EQ(theta7, line);
  std::getline(target_stream, line);
  EXPECT_EQ(message, line);
  target_stream.close();
  int return_code = std::remove(target_csv_file.c_str());
  if (return_code != 0) {
    std::cerr << "Failed to remove file: " << target_csv_file << "\n";
    std::cerr << "Error: " << std::strerror(errno) << std::endl;
    FAIL();
  }
}
