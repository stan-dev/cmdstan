#include <test/utility.hpp>
#include <gtest/gtest.h>

using cmdstan::test::convert_model_path;
using cmdstan::test::run_command;
using cmdstan::test::temporary_unwritable_file;

TEST(interface, fails_when_output_is_unwritable) {
  // makes a file without the w bit set
  temporary_unwritable_file file(
      convert_model_path(std::vector{"test", "output_unwritable.csv"}));

  std::string command = convert_model_path(std::vector{
                            "src", "test", "test-models", "test_model"})
                        + " sample output file=" + file.filename;

  auto out = run_command(command);
  EXPECT_IN_STRING("Permission denied", out.output);
  EXPECT_TRUE(out.hasError);
}

TEST(interface, fails_when_output_is_unwritable_multi) {
  temporary_unwritable_file file1(
      convert_model_path(std::vector{"test", "output_unwritable_1.csv"}));
  temporary_unwritable_file file2(
      convert_model_path(std::vector{"test", "output_unwritable_2.csv"}));

  std::string command = convert_model_path(std::vector{
                            "src", "test", "test-models", "test_model"})
                        + " sample num_chains=2 output file=" + file1.filename
                        + "," + file2.filename;

  auto out = run_command(command);
  EXPECT_IN_STRING("Permission denied", out.output);
  EXPECT_TRUE(out.hasError);
}
