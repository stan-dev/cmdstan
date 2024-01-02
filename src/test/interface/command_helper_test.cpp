#include <cmdstan/command_helper.hpp>
#include <cmdstan/arguments/arg_data.hpp>
#include <cmdstan/arguments/arg_id.hpp>
#include <cmdstan/arguments/arg_init.hpp>
#include <cmdstan/arguments/arg_output.hpp>
#include <cmdstan/arguments/arg_num_threads.hpp>
#include <cmdstan/arguments/arg_random.hpp>
#include <cmdstan/arguments/argument_parser.hpp>
#include <stan/callbacks/stream_writer.hpp>
#include <test/utility.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

using cmdstan::argument;
using cmdstan::argument_parser;
using cmdstan::check_file_config;
using cmdstan::get_basename_suffix;
using cmdstan::get_suffix;
using cmdstan::validate_output_filename;

TEST(CommandHelper, basename_suffix) {
  std::string sep = std::string(1, cmdstan::PATH_SEPARATOR);

  std::string fp1 = "foo" + sep + "bar" + sep + "baz.csv";
  EXPECT_EQ(get_suffix(fp1), ".csv");
  EXPECT_EQ(get_basename_suffix(fp1).first, "foo" + sep + "bar" + sep + "baz");
  EXPECT_EQ(get_basename_suffix(fp1).second, ".csv");

  std::string fp2 = "foo.bar" + sep + "baz";
  EXPECT_EQ(get_suffix(fp2), "");
  EXPECT_EQ(get_basename_suffix(fp2).first, fp2);
  EXPECT_EQ(get_basename_suffix(fp2).second, "");

  std::string fp3 = "foo.bar" + sep + ".." + sep + "baz";
  EXPECT_EQ(get_suffix(fp3), "");
  EXPECT_EQ(get_basename_suffix(fp3).first, fp3);
  EXPECT_EQ(get_basename_suffix(fp3).second, "");

  std::string fp4 = "foo.bar" + sep + ".." + sep + "baz.csv";
  EXPECT_EQ(get_suffix(fp4), ".csv");
  EXPECT_EQ(get_basename_suffix(fp4).first, fp3);
  EXPECT_EQ(get_basename_suffix(fp4).second, ".csv");

  std::string fp5 = "foo.";
  EXPECT_EQ(get_suffix(fp5), "");
  EXPECT_EQ(get_basename_suffix(fp5).first, fp5);
  EXPECT_EQ(get_basename_suffix(fp5).second, "");

  std::string fp6 = "foo.bar" + sep;
  EXPECT_EQ(get_suffix(fp6), "");
  EXPECT_EQ(get_basename_suffix(fp6).first, fp6);
  EXPECT_EQ(get_basename_suffix(fp6).second, "");

  std::string fp7 = "foo.bar.";
  EXPECT_EQ(get_suffix(fp7), ".bar.");
  EXPECT_EQ(get_basename_suffix(fp7).first, "foo");
  EXPECT_EQ(get_basename_suffix(fp7).second, ".bar.");
}

TEST(CommandHelper, validate_output_filename) {
  std::string sep = std::string(1, cmdstan::PATH_SEPARATOR);

  std::string fp1 = "foo.bar";
  EXPECT_NO_THROW(validate_output_filename(fp1));

  std::string fp2 = "foo.bar" + sep;
  EXPECT_THROW(validate_output_filename(fp2), std::invalid_argument);

  std::string fp3 = "foo.bar" + sep + "..";
  EXPECT_THROW(validate_output_filename(fp3), std::invalid_argument);

  std::string fp4 = "foo.bar" + sep + ".";
  EXPECT_THROW(validate_output_filename(fp4), std::invalid_argument);
}

TEST(CommandHelper, check_filename_config_good) {
  std::string sep = std::string(1, cmdstan::PATH_SEPARATOR);

  std::vector<std::string> argv;
  argv.push_back("my_model");
  argv.push_back("sample");
  argv.push_back("output");
  argv.push_back("file=foo" + sep + "bar" + sep + "baz.csv");
  argv.push_back("diagnostic_file=foo" + sep + "bar" + sep + "baz.json");
  char const **argv_prime = new const char *[argv.size()];
  for (size_t i = 0; i < argv.size(); i++) {
    argv_prime[i] = argv[i].c_str();
  }

  std::vector<argument *> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_output());
  argument_parser parser(valid_arguments);
  stan::callbacks::stream_writer info(std::cout);
  stan::callbacks::stream_writer err(std::cerr);
  int err_code = parser.parse_args(argv.size(), argv_prime, info, err);

  EXPECT_NO_THROW(check_file_config(parser));
}

TEST(CommandHelper, check_filename_config_bad) {
  std::string sep = std::string(1, cmdstan::PATH_SEPARATOR);

  std::vector<std::string> argv;
  argv.push_back("my_model");
  argv.push_back("sample");
  argv.push_back("output");
  argv.push_back("file=foo" + sep + "bar" + sep + "baz.csv" + sep);
  char const **argv_prime = new const char *[argv.size()];
  for (size_t i = 0; i < argv.size(); i++) {
    argv_prime[i] = argv[i].c_str();
  }

  std::vector<argument *> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_output());
  argument_parser parser(valid_arguments);
  stan::callbacks::stream_writer info(std::cout);
  stan::callbacks::stream_writer err(std::cerr);
  int err_code = parser.parse_args(argv.size(), argv_prime, info, err);

  EXPECT_THROW(check_file_config(parser), std::invalid_argument);
}

TEST(CommandHelper, check_filename_config_bad2) {
  std::string sep = std::string(1, cmdstan::PATH_SEPARATOR);

  std::vector<std::string> argv;
  argv.push_back("my_model");
  argv.push_back("sample");
  argv.push_back("output");
  argv.push_back("file=foo" + sep + "bar" + sep + "..");
  char const **argv_prime = new const char *[argv.size()];
  for (size_t i = 0; i < argv.size(); i++) {
    argv_prime[i] = argv[i].c_str();
  }

  std::vector<argument *> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_output());
  argument_parser parser(valid_arguments);
  stan::callbacks::stream_writer info(std::cout);
  stan::callbacks::stream_writer err(std::cerr);
  int err_code = parser.parse_args(argv.size(), argv_prime, info, err);

  EXPECT_THROW(check_file_config(parser), std::invalid_argument);
}

TEST(CommandHelper, check_filename_config_bad3) {
  std::string sep = std::string(1, cmdstan::PATH_SEPARATOR);

  std::vector<std::string> argv;
  argv.push_back("my_model");
  argv.push_back("sample");
  argv.push_back("output");
  argv.push_back("file=foo.bar." + sep);
  char const **argv_prime = new const char *[argv.size()];
  for (size_t i = 0; i < argv.size(); i++) {
    argv_prime[i] = argv[i].c_str();
  }

  std::vector<argument *> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_output());
  argument_parser parser(valid_arguments);
  stan::callbacks::stream_writer info(std::cout);
  stan::callbacks::stream_writer err(std::cerr);
  int err_code = parser.parse_args(argv.size(), argv_prime, info, err);

  EXPECT_THROW(check_file_config(parser), std::invalid_argument);
}
