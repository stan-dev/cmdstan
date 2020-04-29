#include <cmdstan/arguments/singleton_argument.hpp>
#include <stan/callbacks/stream_writer.hpp>
#include <stan/callbacks/writer.hpp>
#include <gtest/gtest.h>

template <typename T>
T argument_value() {
  return 0;
}

template <typename T>
std::string argument_string() {
  return boost::lexical_cast<std::string>(argument_value<T>());
}

template <>
double argument_value<double>() {
  return 1.234;
}

template <>
int argument_value<int>() {
  return 567;
}

template <>
bool argument_value<bool>() {
  return true;
}

template <>
std::string argument_value<std::string>() {
  return "value";
}

template <typename T>
class CmdStanArgumentsSingleton : public ::testing::Test {
 public:
  CmdStanArgumentsSingleton()
      : arg(new cmdstan::singleton_argument<T>("argument")) {}

  virtual ~CmdStanArgumentsSingleton() { delete (arg); }

  cmdstan::argument *arg;
  std::stringstream ss;
};

TYPED_TEST_CASE_P(CmdStanArgumentsSingleton);

TYPED_TEST_P(CmdStanArgumentsSingleton, constructor) {
  // test fixture would have created the argument
}

TYPED_TEST_P(CmdStanArgumentsSingleton, name) {
  EXPECT_EQ("argument", this->arg->name());
}

TYPED_TEST_P(CmdStanArgumentsSingleton, description) {
  EXPECT_EQ("", this->arg->description());
}

TYPED_TEST_P(CmdStanArgumentsSingleton, print) {
  // FIXME: write test
}

TYPED_TEST_P(CmdStanArgumentsSingleton, print_help) {
  // FIXME: write test
}

TYPED_TEST_P(CmdStanArgumentsSingleton, parse_args) {
  bool return_value;
  std::vector<std::string> args;
  bool help_flag;
  stan::callbacks::stream_writer out(this->ss);
  stan::callbacks::writer err;

  return_value = false;
  args.clear();
  help_flag = false;
  return_value = this->arg->parse_args(args, out, err, help_flag);

  EXPECT_TRUE(return_value);
  EXPECT_FALSE(help_flag);
  EXPECT_EQ(0U, args.size());

  return_value = false;
  args.clear();
  args.push_back("help");
  help_flag = false;
  return_value = this->arg->parse_args(args, out, err, help_flag);

  EXPECT_TRUE(return_value);
  EXPECT_TRUE(help_flag);
  EXPECT_EQ(0U, args.size());

  return_value = false;
  args.clear();
  args.push_back("help-all");
  help_flag = false;
  return_value = this->arg->parse_args(args, out, err, help_flag);

  EXPECT_TRUE(return_value);
  EXPECT_TRUE(help_flag);
  EXPECT_EQ(0U, args.size());

  return_value = false;
  args.clear();
  args.push_back("argument=" + argument_string<TypeParam>());
  help_flag = false;
  return_value = this->arg->parse_args(args, out, err, help_flag);

  EXPECT_TRUE(return_value);
  EXPECT_FALSE(help_flag);
  EXPECT_EQ(0U, args.size());
  EXPECT_EQ(argument_value<TypeParam>(),
            static_cast<cmdstan::singleton_argument<TypeParam> *>(this->arg)
                ->value());
}

TYPED_TEST_P(CmdStanArgumentsSingleton, parse_args_unexpected) {
  bool return_value;
  std::vector<std::string> args;
  bool help_flag;
  stan::callbacks::stream_writer out(this->ss);
  stan::callbacks::writer err;

  return_value = false;
  args.clear();
  args.push_back("foo=bar");
  help_flag = false;
  return_value = this->arg->parse_args(args, out, err, help_flag);

  EXPECT_TRUE(return_value);
  EXPECT_FALSE(help_flag);
  EXPECT_EQ(1U, args.size());
}

TYPED_TEST_P(CmdStanArgumentsSingleton, argument_lookup) {
  // EXPECT_EQ(0, this->arg->arg(""));
  // EXPECT_EQ(0, this->arg->arg("foo"));
}

REGISTER_TYPED_TEST_CASE_P(CmdStanArgumentsSingleton, constructor, name,
                           description, print, print_help, parse_args,
                           parse_args_unexpected, argument_lookup);

INSTANTIATE_TYPED_TEST_CASE_P(real, CmdStanArgumentsSingleton, double);
INSTANTIATE_TYPED_TEST_CASE_P(int, CmdStanArgumentsSingleton, int);
INSTANTIATE_TYPED_TEST_CASE_P(bool, CmdStanArgumentsSingleton, bool);
INSTANTIATE_TYPED_TEST_CASE_P(string, CmdStanArgumentsSingleton, std::string);
