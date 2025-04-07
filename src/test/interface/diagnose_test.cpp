#include <test/utility.hpp>
#include <gtest/gtest.h>
#include <fstream>
#include <sstream>

using cmdstan::test::compare_to_stored_output;
using cmdstan::test::count_matches;
using cmdstan::test::get_path_separator;
using cmdstan::test::run_command;

TEST(CommandDiagnose, corr_gauss) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "diagnose";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "corr_gauss_output.csv";

  auto out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  compare_to_stored_output(out.output,
                           "src/test/interface/example_output/corr_gauss.nom");
}

TEST(CommandDiagnose, corr_gauss_depth8) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "diagnose";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "corr_gauss_output_depth8.csv";

  auto out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  compare_to_stored_output(
      out.output, "src/test/interface/example_output/corr_gauss_depth8.nom");
}

TEST(CommandDiagnose, corr_gauss_depth15) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "diagnose";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "corr_gauss_output_depth15.csv";

  auto out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  compare_to_stored_output(
      out.output, "src/test/interface/example_output/corr_gauss_depth15.nom");
}

TEST(CommandDiagnose, eight_schools) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "diagnose";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "eight_schools_output.csv";

  auto out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  compare_to_stored_output(
      out.output, "src/test/interface/example_output/eight_schools.nom");
}

TEST(CommandDiagnose, mix) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "diagnose";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "mix_output.*";

  auto out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  compare_to_stored_output(out.output,
                           "src/test/interface/example_output/mix.nom");
}

TEST(CommandDiagnose, divergences) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "bin" + path_separator + "diagnose";
  std::string csv_file = "src" + path_separator + "test" + path_separator
                         + "interface" + path_separator + "example_output"
                         + path_separator + "div_output*.csv";

  auto out = run_command(command + " " + csv_file);
  ASSERT_FALSE(out.hasError) << "\"" << out.command << "\" quit with an error";

  compare_to_stored_output(out.output,
                           "src/test/interface/example_output/div.nom");
}
