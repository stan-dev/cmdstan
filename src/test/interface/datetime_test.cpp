#include <stan/mcmc/chains.hpp>
#include <stan/services/error_codes.hpp>
#include <test/utility.hpp>
#include <gtest/gtest.h>
#include <ctime>
#include <fstream>

using cmdstan::test::convert_model_path;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

TEST(interface, csv_header_consistency) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("test_model");

  std::string path = convert_model_path(model_path);
  std::string samples = path + ".csv";

  std::string command
      = path + " sample num_warmup=1 num_samples=1" + " output file=" + samples;

  run_command_output out = run_command(command);
  EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code);
  EXPECT_FALSE(out.hasError);

  std::ifstream in;
  in.open(samples.c_str());

  std::stringstream ss;
  std::string line;

  while (in.peek() == '#') {
    std::getline(in, line);
    ss << line << '\n';
  }
  ss.seekg(std::ios_base::beg);

  char comment;
  std::string lhs;

  std::string name;
  std::string value;

  while (ss.good()) {
    ss >> comment;
    std::getline(ss, lhs);
    size_t equal = lhs.find("=");
    if (equal != std::string::npos) {
      name = lhs.substr(0, equal);
      boost::trim(name);
      if (name.compare("start_datetime") == 0) {
        value = lhs.substr(equal + 1, lhs.size());
        boost::trim(value);
        break;
      }
    }
  }
  EXPECT_EQ(value.size(), 23);
  time_t theTime = time(NULL);
  struct tm *aTime = std::gmtime(&theTime);

  EXPECT_EQ(std::atoi(value.substr(0, 4).c_str()), (aTime->tm_year + 1900));
  EXPECT_EQ(value.substr(value.size() - 3, 3), "UTC");
  in.close();
}
