#include <cmdstan/arguments/arg_data.hpp>
#include <cmdstan/arguments/arg_id.hpp>
#include <cmdstan/arguments/arg_init.hpp>
#include <cmdstan/arguments/arg_output.hpp>
#include <cmdstan/arguments/arg_random.hpp>
#include <cmdstan/arguments/argument_parser.hpp>
#include <stan/callbacks/writer.hpp>
#include <stan/callbacks/stream_writer.hpp>
#include <boost/algorithm/string.hpp>
#include <stan/services/error_codes.hpp>
#include <gtest/gtest.h>

using cmdstan::arg_data;
using cmdstan::arg_id;
using cmdstan::arg_init;
using cmdstan::arg_output;
using cmdstan::arg_random;
using cmdstan::argument;
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
    delete (parser);
  }

  /**
   * Given a suggestion from the parser, massage it into
   * a form that looks like a command line input `argv`
   * and then parse it and assert suggestion is accepted.
   *
   * Currently, this only replaces the `"<type>"` field
   * which can occur in a suggestion with a value for
   * double arguments, but it could be easily extended.
   */
  void check_suggestion(std::string suggestion) {
    boost::trim(suggestion);
    boost::replace_first(suggestion, "<double>",
                         "1e-20");  // replace type with value
    std::vector<std::string> args;
    boost::split(args, suggestion, boost::is_any_of(" "));
    args.insert(args.begin(), "mymodel");  // add model name

    // convert to char** for parse_args
    char const **argv_sug = new const char *[args.size()];
    for (size_t i = 0; i < args.size(); i++) {
      argv_sug[i] = args[i].c_str();
    }

    err_code = parser->parse_args(args.size(), argv_sug, writer, writer);
    EXPECT_EQ(int(error_codes::OK), err_code);
  }

  std::vector<argument *> valid_arguments;
  argument_parser *parser;
  int err_code;
  stan::callbacks::writer writer;
};

TEST_F(CmdStanArgumentsArgumentParser, default) {
  const char *argv[] = {};
  int argc = 0;

  err_code = parser->parse_args(argc, argv, writer, writer);
  EXPECT_EQ(int(error_codes::USAGE), err_code);
}

TEST_F(CmdStanArgumentsArgumentParser, help) {
  const char *argv[] = {"model_name", "help"};
  int argc = 2;

  err_code = parser->parse_args(argc, argv, writer, writer);
  EXPECT_EQ(int(error_codes::OK), err_code);
}

TEST_F(CmdStanArgumentsArgumentParser, unrecognized_argument) {
  const char *argv[] = {"foo", "bar"};
  int argc = 2;

  err_code = parser->parse_args(argc, argv, writer, writer);
  EXPECT_EQ(int(error_codes::USAGE), err_code);
}

TEST_F(CmdStanArgumentsArgumentParser, find_args) {
  const char *argv[] = {"foo", "tol_grad=1e-20"};
  int argc = 2;
  std::stringstream err_stream;
  stan::callbacks::stream_writer err_writer(err_stream);

  err_code = parser->parse_args(argc, argv, writer, err_writer);
  EXPECT_EQ(int(error_codes::USAGE), err_code);

  std::string out;
  std::getline(err_stream,
               out);  // skip "tol_grad=1e-20 is either mistyped or misplaced."
  std::getline(err_stream, out);  // skip "Perhaps you meant one of the
                                  // following valid configurations?"

  std::getline(err_stream, out);  // first suggestion
  check_suggestion(out);
  std::getline(err_stream, out);  // second suggestion
  check_suggestion(out);
}
