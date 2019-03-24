#include <fstream>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <test/utility.hpp>
#include <stan/io/stan_csv_reader.hpp>

using cmdstan::test::convert_model_path;
using cmdstan::test::get_path_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
public:
  void SetUp() {
    std::string path_separator;
    path_separator.push_back(get_path_separator());
    command = "bin" + path_separator + "collate";
    test_dir = { "src", "test", "test-models"};
    null_output_file_path = { "/dev", "null"};
    output_file_path = { "test", "collated.csv"};
  }

  std::string command;
  std::vector<std::string> test_dir;
  std::vector<std::string> null_output_file_path;
  std::vector<std::string> output_file_path;
};


TEST_F(CmdStan, collate_good_1) {
  std::stringstream ss;
  std::vector<std::string> input_path(test_dir);
  input_path.emplace_back("bernoulli_*.csv");
  ss << command
     <<  " --collate_csv_file=" << convert_model_path(null_output_file_path)
     << " " << convert_model_path(input_path);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  EXPECT_EQ(out.output, "");
}

TEST_F(CmdStan, collate_good_2) {
  std::stringstream ss;
  std::string csv_out = convert_model_path(output_file_path);
  ss << command << " --collate_csv_file=" << csv_out;

  std::ifstream ifstream;
  std::vector<std::string> input_path(test_dir);
  stan::io::stan_csv stan_csv;
  size_t num_draws = 0;

  input_path.emplace_back("bernoulli_1.csv");
  std::string csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  ifstream.open(csv_in.c_str());
  stan_csv = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
  ifstream.close();
  num_draws += stan_csv.samples.rows();
  
  input_path[input_path.size() - 1] =  "bernoulli_2.csv";
  csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  ifstream.open(csv_in.c_str());
  stan_csv = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
  ifstream.close();
  num_draws += stan_csv.samples.rows();
  
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  EXPECT_EQ(out.output, "");

  ifstream.open(csv_out.c_str());
  stan_csv = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
  ifstream.close();
  EXPECT_EQ(stan_csv.samples.rows(), num_draws);
}

TEST_F(CmdStan, collate_bad_1) {
  std::stringstream ss;
  std::vector<std::string> input_path(test_dir);
  input_path.emplace_back("no_such_file");
  ss << command
     <<  " --collate_csv_file=" << convert_model_path(null_output_file_path)
     << " " << convert_model_path(input_path);

  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
  EXPECT_EQ(out.output, "File src/test/test-models/no_such_file not found\nNo valid input files, exiting.\n");
}

TEST_F(CmdStan, collate_bad_2) {
  std::stringstream ss;
  std::string csv_out = convert_model_path(output_file_path);
  ss << command << " --collate_csv_file=" << csv_out;

  std::vector<std::string> input_path(test_dir);
  std::string csv_in;
  input_path.emplace_back("bernoulli_1.csv");
  csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  
  input_path[input_path.size() - 1] =  "proper.csv";
  csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
  EXPECT_EQ(out.output,
            "Error, file: src/test/test-models/proper.csv, expecting sample from model bernoulli_model, found sample from model proper_model, exiting.\n");
}

TEST_F(CmdStan, collate_bad_3) {
  std::stringstream ss;
  std::string csv_out = convert_model_path(output_file_path);
  ss << command << " --collate_csv_file=" << csv_out;

  std::vector<std::string> input_path(test_dir);
  std::string csv_in;
  input_path.emplace_back("bernoulli_1.csv");
  csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  
  input_path[input_path.size() - 1] =  "collate_bad_names.csv";
  csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
  EXPECT_EQ(out.output,
            "Error, file: src/test/test-models/collate_bad_names.csv, column header names mismatch, column 7, expecting name theta, found name gamma, exiting.\n");
}

TEST_F(CmdStan, collate_bad_4) {
  std::stringstream ss;
  std::string csv_out = convert_model_path(output_file_path);
  ss << command << " --collate_csv_file=" << csv_out;

  std::vector<std::string> input_path(test_dir);
  std::string csv_in;
  input_path.emplace_back("bernoulli_1.csv");
  csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  
  input_path[input_path.size() - 1] =  "collate_bad_columns.csv";
  csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
  EXPECT_EQ(out.output,
            "Error, file: src/test/test-models/collate_bad_columns.csv, wrong number of output columns, expecting 8 columns, found 9 columns, exiting.\n");
}
