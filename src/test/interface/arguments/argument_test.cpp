#include <cmdstan/arguments/argument.hpp>
#include <stan/callbacks/writer.hpp>
#include <gtest/gtest.h>

class test_arg_impl : public cmdstan::argument {
  void print(stan::callbacks::writer &w, int depth, const std::string &prefix) {
  }
  void print(stan::callbacks::structured_writer &j) {}
  void print_help(stan::callbacks::writer &w, int depth, bool recurse) {}
};

class CmdStanArgumentsArgument : public testing::Test {
 public:
  void SetUp() { arg = new test_arg_impl; }
  void TearDown() { delete (arg); }

  cmdstan::argument *arg;
};

TEST_F(CmdStanArgumentsArgument, Constructor) {
  // test fixture would have created the argument.
}

TEST_F(CmdStanArgumentsArgument, name) { EXPECT_EQ("", arg->name()); }

TEST_F(CmdStanArgumentsArgument, description) {
  EXPECT_EQ("", arg->description());
}

TEST_F(CmdStanArgumentsArgument, split_arg) {
  std::string arg_string;
  std::string name;
  std::string value;

  arg_string = "";
  arg->split_arg(arg_string, name, value);
  EXPECT_EQ("", name);
  EXPECT_EQ("", value);

  arg_string = "foo=bar";
  arg->split_arg(arg_string, name, value);
  EXPECT_EQ("foo", name);
  EXPECT_EQ("bar", value);

  arg_string = " foo=bar ";
  arg->split_arg(arg_string, name, value);
  EXPECT_EQ(" foo", name);
  EXPECT_EQ("bar ", value);

  arg_string = "\nfoo=\rbar\t";
  arg->split_arg(arg_string, name, value);
  EXPECT_EQ("\nfoo", name);
  EXPECT_EQ("\rbar\t", value);
}
