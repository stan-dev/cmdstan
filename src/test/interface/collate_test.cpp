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
    model_path = {"src", "test", "test-models", "gq_model"};
    data_file_path = {"src", "test", "test-models", "gq_model.data.json"};
  }

  std::string command;
  std::vector<std::string> test_dir;
  std::vector<std::string> null_output_file_path;
  std::vector<std::string> output_file_path;
  std::vector<std::string> model_path;
  std::vector<std::string> data_file_path;
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

TEST_F(CmdStan, collate_good_3) {
  std::stringstream ss;
  std::string csv_out = convert_model_path(output_file_path);
  ss << command << " --collate_csv_file=" << csv_out;

  std::ifstream ifstream;
  std::vector<std::string> input_path(test_dir);
  stan::io::stan_csv stan_csv;
  size_t num_draws = 0;
  std::vector<size_t> draws_per_chain;
  
  input_path.emplace_back("bernoulli_1.csv");
  std::string csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  ifstream.open(csv_in.c_str());
  stan_csv = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
  ifstream.close();
  num_draws += stan_csv.samples.rows();
  draws_per_chain.emplace_back(stan_csv.samples.rows());
  
  input_path[input_path.size() - 1] =  "bernoulli_2.csv";
  csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  ifstream.open(csv_in.c_str());
  stan_csv = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
  ifstream.close();
  num_draws += stan_csv.samples.rows();
  draws_per_chain.emplace_back(stan_csv.samples.rows());
  
  input_path[input_path.size() - 1] =  "bernoulli_3.csv";
  csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  ifstream.open(csv_in.c_str());
  stan_csv = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
  ifstream.close();
  num_draws += stan_csv.samples.rows();
  draws_per_chain.emplace_back(stan_csv.samples.rows());
  
  input_path[input_path.size() - 1] =  "bernoulli_4.csv";
  csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  ifstream.open(csv_in.c_str());
  stan_csv = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
  ifstream.close();
  num_draws += stan_csv.samples.rows();
  draws_per_chain.emplace_back(stan_csv.samples.rows());
  
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
  EXPECT_EQ(out.output, "");

  ifstream.open(csv_out.c_str());
  stan_csv = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
  ifstream.close();
  EXPECT_EQ(stan_csv.samples.rows(), num_draws);

  ifstream.open(csv_out.c_str());
  std::string line;
  // comments
  while (ifstream.peek() == '#')
    std::getline(ifstream, line);
  // header
  while (ifstream.peek() != '#')
    std::getline(ifstream, line);
  // adaptation
  while (ifstream.peek() == '#')
    std::getline(ifstream, line);
  // sample
  while (ifstream.peek() != '#')
    std::getline(ifstream, line);

  // chain ids
  std::getline(ifstream, line);
  EXPECT_EQ(line, "# chain_ids: 1, 2, 3, 4");

  // chain draws
  std::getline(ifstream, line);
  EXPECT_EQ(line, "# chain_draws: 500, 500, 100, 5");
  ifstream.close();
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
  
  input_path[input_path.size() - 1] =  "collate_bad_model.csv";
  csv_in = convert_model_path(input_path);
  ss << " " << csv_in;
  
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
  EXPECT_EQ(out.output,
            "Error, file: src/test/test-models/collate_bad_model.csv, expecting sample from model bernoulli_model, found sample from model proper_model, exiting.\n");
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


TEST_F(CmdStan, collate_generate) {
  std::stringstream ss;
  std::string csv_out = convert_model_path(output_file_path);

  std::vector<std::string> input_path(test_dir);
  input_path.emplace_back("bernoulli_*.csv");
  ss << command << " --collate_csv_file=" << csv_out
     << " " << convert_model_path(input_path);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);

  ASSERT_FALSE(out.hasError);
  EXPECT_EQ(out.output, "");

  ss.str(std::string());
  ss << convert_model_path(model_path)
     << " data file=" << convert_model_path(data_file_path)
     << " output file=" << convert_model_path(null_output_file_path)
     << " method=generate_quantities fitted_params=" << csv_out;
  cmd = ss.str();
  out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}
