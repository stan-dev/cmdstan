#include <gtest/gtest.h>
#include <test/utility.hpp>
#include <fstream>
#include <streambuf>

TEST(CommandElapsedTime, PrintToScreen) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "src" + path_separator + "test" 
    + path_separator + "test-models" + path_separator + "test_model"
    + " sample output file=test/output.csv";

  run_command_output out = run_command(command);
  ASSERT_FALSE(out.hasError) 
    << "\"" << out.command << "\" quit with an error";

  EXPECT_EQ(1, count_matches("#  Elapsed Time: ", out.output))
    << "output stream should have one match for elapsed time";
}

TEST(CommandElapsedTime, PrintToFile) {
  std::string path_separator;
  path_separator.push_back(get_path_separator());
  std::string command = "src" + path_separator + "test" 
    + path_separator + "test-models" + path_separator + "test_model"
    + " sample output file=test/output.csv";

  run_command_output out = run_command(command);
  ASSERT_FALSE(out.hasError) 
    << "\"" << out.command << "\" quit with an error";
  std::ifstream ifstream("test/output.csv");
  std::string output_file((std::istreambuf_iterator<char>(ifstream)),
                           std::istreambuf_iterator<char>());
  ifstream.close();
                          
  EXPECT_EQ(1, count_matches("#  Elapsed Time: ", output_file))
    << "output_file should have one match for elapsed time";
}
