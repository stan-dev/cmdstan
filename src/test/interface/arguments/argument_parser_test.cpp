#include <cmdstan/arguments/argument_parser.hpp>
#include <cmdstan/arguments/arg_id.hpp>
#include <cmdstan/arguments/arg_data.hpp>
#include <cmdstan/arguments/arg_init.hpp>
#include <cmdstan/arguments/arg_random.hpp>
#include <cmdstan/arguments/arg_output.hpp>
#include <stan/callbacks/writer.hpp>
#include <stan/services/error_codes.hpp>
#include <gtest/gtest.h>

using cmdstan::argument;
using cmdstan::arg_id;
using cmdstan::arg_data;
using cmdstan::arg_init;
using cmdstan::arg_random;
using cmdstan::arg_output;
using cmdstan::argument_parser;
using stan::services::error_codes;

class CmdStanArgumentsArgumentParser : public testing::Test {
public:
  void SetUp() {
    // copied setup from src/stan/common/command.hpp
    // FIXME: move to factory?
    valid_arguments.push_back(new arg_id());
    valid_arguments.push_back(new arg_data());
    valid_arguments.push_back(new arg_init());
    valid_arguments.push_back(new arg_random());
    valid_arguments.push_back(new arg_output());

    parser = new argument_parser(valid_arguments);
  }
  void TearDown() {
    for (size_t i = 0; i < valid_arguments.size(); ++i)
      delete valid_arguments.at(i);
    delete(parser);
  }

  std::vector<argument*> valid_arguments;
  argument_parser* parser;
  int err_code;
  stan::callbacks::writer writer;
};


TEST_F(CmdStanArgumentsArgumentParser, default) {
  const char* argv[] = {};
  int argc = 0;

  err_code = parser->parse_args(argc, argv, writer, writer);
  EXPECT_EQ(int(error_codes::USAGE), err_code);
}

TEST_F(CmdStanArgumentsArgumentParser, help) {
  const char* argv[] = {"model_name", "help"};
  int argc = 2;

  err_code = parser->parse_args(argc, argv, writer, writer);
  EXPECT_EQ(int(error_codes::OK), err_code);
}

TEST_F(CmdStanArgumentsArgumentParser, unrecognized_argument) {
  const char* argv[] = {"foo"};
  int argc = 1;

  err_code = parser->parse_args(argc, argv, writer, writer);
  EXPECT_EQ(int(error_codes::USAGE), err_code);
}
