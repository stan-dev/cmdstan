#include <cmdstan/arguments/unvalued_argument.hpp>
#include <stan/callbacks/stream_writer.hpp>
#include <stan/callbacks/writer.hpp>
#include <gtest/gtest.h>

class test_arg_impl : public cmdstan::unvalued_argument {
  std::string print_value() {
    return "";
  }
  std::string print_valid() {
    return "";
  }
  bool is_default() {
    return true;
  }
};


class CmdStanArgumentsUnvaluedArgument : public testing::Test {
public:
  void SetUp () {
    arg = new test_arg_impl;
  }
  void TearDown() {
    delete(arg);
  }

  cmdstan::argument *arg;
  std::stringstream ss;
};


TEST_F(CmdStanArgumentsUnvaluedArgument,Constructor) {
  // test fixture would have created the argument.
}

TEST_F(CmdStanArgumentsUnvaluedArgument,name) {
  EXPECT_EQ("", arg->name());
}

TEST_F(CmdStanArgumentsUnvaluedArgument,description) {
  EXPECT_EQ("", arg->description());
}

TEST_F(CmdStanArgumentsUnvaluedArgument,print) {
  // FIXME: write test
}

TEST_F(CmdStanArgumentsUnvaluedArgument,print_help) {
  // FIXME: write test
}

TEST_F(CmdStanArgumentsUnvaluedArgument,parse_args) {
  bool return_value;
  std::vector<std::string> args;
  bool help_flag;
  stan::callbacks::stream_writer out(ss);
  stan::callbacks::writer err;

  return_value = false;
  args.clear();
  help_flag = false;
  return_value = arg->parse_args(args,out,err,help_flag);

  EXPECT_TRUE(return_value);
  EXPECT_FALSE(help_flag);
  EXPECT_EQ(0U, args.size());


  return_value = false;
  args.clear();
  args.push_back("help");
  help_flag = false;
  return_value = arg->parse_args(args,out,err,help_flag);

  EXPECT_TRUE(return_value);
  EXPECT_TRUE(help_flag);
  EXPECT_EQ(0U, args.size());
  EXPECT_FALSE(static_cast<test_arg_impl*>(arg)->is_present());


  return_value = false;
  args.clear();
  args.push_back("help-all");
  help_flag = false;
  return_value = arg->parse_args(args,out,err,help_flag);

  EXPECT_TRUE(return_value);
  EXPECT_TRUE(help_flag);
  EXPECT_EQ(0U, args.size());
  EXPECT_FALSE(static_cast<test_arg_impl*>(arg)->is_present());
}

TEST_F(CmdStanArgumentsUnvaluedArgument,parse_args_unexpected) {
  bool return_value;
  std::vector<std::string> args;
  bool help_flag;
  stan::callbacks::stream_writer out(ss);
  stan::callbacks::writer err;

  return_value = false;
  args.clear();
  args.push_back("foo=bar");
  help_flag = false;
  return_value = arg->parse_args(args,out,err,help_flag);

  EXPECT_TRUE(return_value);
  EXPECT_FALSE(help_flag);
  EXPECT_EQ(1U, args.size());
  EXPECT_TRUE(static_cast<test_arg_impl*>(arg)->is_present());
}

TEST_F(CmdStanArgumentsUnvaluedArgument,arg) {
  EXPECT_EQ(0, arg->arg(""));
  EXPECT_EQ(0, arg->arg("foo"));
}
