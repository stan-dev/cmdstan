#include <test/utility.hpp>
#include <gtest/gtest.h>
#include <fstream>
#include <sstream>

using cmdstan::test::count_matches;
using cmdstan::test::get_path_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

TEST(CommandDiagnose, corr_gauss) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "diagnose";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "corr_gauss_output.csv";

  run_command_output out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  std::ifstream expected_output(
      "src/test/interface/example_output/corr_gauss.nom");
  std::stringstream ss;
  ss << expected_output.rdbuf();
  EXPECT_EQ(1, count_matches(ss.str(), out.output));
}

TEST(CommandDiagnose, corr_gauss_depth8) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "diagnose";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "corr_gauss_output_depth8.csv";

  run_command_output out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  std::ifstream expected_output(
      "src/test/interface/example_output/corr_gauss_depth8.nom");
  std::stringstream ss;
  ss << expected_output.rdbuf();
  EXPECT_EQ(1, count_matches(ss.str(), out.output));
}

TEST(CommandDiagnose, corr_gauss_depth15) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "diagnose";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "corr_gauss_output_depth15.csv";

  run_command_output out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  std::ifstream expected_output(
      "src/test/interface/example_output/corr_gauss_depth15.nom");
  std::stringstream ss;
  ss << expected_output.rdbuf();
  EXPECT_EQ(1, count_matches(ss.str(), out.output));
}

TEST(CommandDiagnose, eight_schools) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "diagnose";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "eight_schools_output.csv";

  run_command_output out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  std::ifstream expected_output(
      "src/test/interface/example_output/eight_schools.nom");
  std::stringstream ss;
  ss << expected_output.rdbuf();
  EXPECT_EQ(1, count_matches(ss.str(), out.output));
}

TEST(CommandDiagnose, mix) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "diagnose";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "mix_output.*";

  run_command_output out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  std::ifstream expected_output("src/test/interface/example_output/mix.nom");
  std::stringstream ss;
  ss << expected_output.rdbuf();
  EXPECT_EQ(1, count_matches(ss.str(), out.output));
}
