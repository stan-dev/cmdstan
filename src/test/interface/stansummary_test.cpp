#include <cmdstan/stansummary_helper.hpp>
#include <test/utility.hpp>
#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>

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

TEST(CommandStansummary, functional_test__issue_342) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "matrix_output.csv";

  run_command_output out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";
}

TEST(CommandStansummary, missing_input_files) {
  std::string expected_message = "No Stan csv file(s) specified";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  run_command_output out = run_command(command);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";
}

TEST(CommandStansummary, bad_input_files) {
  std::string expected_message = "Invalid input file";
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
  std::string expected_message = "Invalid output csv file";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";
  std::string arg_csv_file
      = "--csv_file=" + path_separator + "bin" + path_separator + "hi_mom.csv";

  run_command_output out
      = run_command(command + " " + arg_csv_file + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";
}

TEST(CommandStansummary, bad_sig_figs_arg) {
  std::string expected_message = "Bad value for option --sig_figs";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";
  std::string arg_sig_figs = "--sig_figs=-1";

  run_command_output out
      = run_command(command + " " + arg_sig_figs + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";

  arg_sig_figs = "--sig_figs=101";
  out = run_command(command + " " + arg_sig_figs + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";
}

TEST(CommandStansummary, bad_autocorr_arg) {
  std::string expected_message = "Bad value for option --autocorr";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";
  std::string arg_autocorr = "--autocorr=-1";

  run_command_output out
      = run_command(command + " " + arg_autocorr + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";

  arg_autocorr = "--autocorr=0";
  out = run_command(command + " " + arg_autocorr + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";

  arg_autocorr = "--autocorr=2";
  out = run_command(command + " " + arg_autocorr + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";
}

TEST(CommandStansummary, bad_percentiles_arg) {
  std::string expected_message = "option --percentiles";
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "stansummary";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "bernoulli_chain_1.csv";
  std::string arg_percentiles = "--percentiles=-1";

  run_command_output out
      = run_command(command + " " + arg_percentiles + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";

  arg_percentiles = "--percentiles=\"0,100\"";
  out = run_command(command + " " + arg_percentiles + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";

  arg_percentiles = "--percentiles=\"2,30,5\"";
  out = run_command(command + " " + arg_percentiles + " " + csv_file);
  EXPECT_TRUE(boost::algorithm::contains(out.output, expected_message));
  ASSERT_TRUE(out.hasError)
      << "\"" << out.command << "\" failed to quit with an error";

  arg_percentiles = "--percentiles=\"2,50,95\"";
  out = run_command(command + " " + arg_percentiles + " " + csv_file);
  ASSERT_FALSE(out.hasError)
      << "\"" << out.command << "\" quit with an error";
}
