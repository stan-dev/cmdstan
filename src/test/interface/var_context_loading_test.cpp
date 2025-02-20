#include <cmdstan/command_helper.hpp>
#include <gtest/gtest.h>
#include <test/utility.hpp>

#include <string>
#include <vector>

class VarContextLoading : public testing::Test {
 public:
  void SetUp() {
    using cmdstan::test::convert_model_path;
    init_good = convert_model_path(
        std::vector{"src", "test", "test-models", "bern_init.json"});
    init_template_good = convert_model_path(
        std::vector{"src", "test", "test-models", "bern_init2.json"});
    init_template_good_member = convert_model_path(
        std::vector{"src", "test", "test-models", "bern_init2_2.json"});
    init_template_missing_member = convert_model_path(
        std::vector{"src", "test", "test-models", "bern_init2_6.json"});
    init_template_R_good = convert_model_path(
        std::vector{"src", "test", "test-models", "bern_init2.R"});
    init_template_R_good_member = convert_model_path(
        std::vector{"src", "test", "test-models", "bern_init2_2.R"});
  }
  std::string init_good;
  std::string init_template_good;
  std::string init_template_good_member;
  std::string init_template_missing_member;
  std::string init_template_R_good;
  std::string init_template_R_good_member;
};

// --------------- single chain ---------------

TEST_F(VarContextLoading, one_chain_good) {
  auto var_context = cmdstan::get_vec_var_context(init_good, 1, 1);
  EXPECT_EQ(1, var_context.size());
  EXPECT_FLOAT_EQ(0.1, var_context[0]->vals_r("theta")[0]);
}

TEST_F(VarContextLoading, one_chain_bad_extension) {
  EXPECT_THROW_MSG(
      cmdstan::get_vec_var_context(init_good + "foo", 1, 1),
      std::invalid_argument,
      "User specified files must end in .json or .R. Found: .jsonfoo");
}

TEST_F(VarContextLoading, one_chain_missing_file) {
  EXPECT_THROW_MSG(
      cmdstan::get_vec_var_context(init_template_missing_member, 1, 1),
      std::invalid_argument, "Cannot open specified file");
}

TEST_F(VarContextLoading, one_chain_warning_R_extension) {
  testing::internal::CaptureStderr();
  auto var_context
      = cmdstan::get_vec_var_context(init_template_R_good_member, 1, 1);
  std::string output = testing::internal::GetCapturedStderr();
  EXPECT_IN_STRING(
      "This format is deprecated and will not receive new features.", output);
  EXPECT_EQ(1, var_context.size());
  EXPECT_FLOAT_EQ(0.2, var_context[0]->vals_r("theta")[0]);
}

TEST_F(VarContextLoading, one_chain_empty) {
  auto var_context = cmdstan::get_vec_var_context("", 1, 1);
  EXPECT_EQ(1, var_context.size());
  std::vector<std::string> names;
  var_context[0]->names_r(names);
  EXPECT_EQ(0, names.size());
}

// --------------- multi-chain, legacy 'template' format ---------------

TEST_F(VarContextLoading, multi_template_good) {
  auto var_context = cmdstan::get_vec_var_context(init_template_good, 4, 1);
  EXPECT_EQ(4, var_context.size());
  EXPECT_FLOAT_EQ(0.1, var_context[0]->vals_r("theta")[0]);
  EXPECT_FLOAT_EQ(0.2, var_context[1]->vals_r("theta")[0]);
}

TEST_F(VarContextLoading, multi_template_fallback) {
  testing::internal::CaptureStderr();
  auto var_context = cmdstan::get_vec_var_context(init_good, 4, 1);
  std::string output = testing::internal::GetCapturedStderr();
  EXPECT_IN_STRING("is being used to initialize all 4 chains", output);

  EXPECT_EQ(4, var_context.size());
  // all the same
  for (size_t i = 0; i < 4; ++i) {
    EXPECT_FLOAT_EQ(0.1, var_context[i]->vals_r("theta")[0]);
  }
}

TEST_F(VarContextLoading, multi_template_bad_extension) {
  EXPECT_THROW_MSG(
      cmdstan::get_vec_var_context(init_template_good + "foo", 4, 1),
      std::invalid_argument,
      "User specified files must end in .json or .R. Found: .jsonfoo");
}

TEST_F(VarContextLoading, multi_template_missing_file) {
  EXPECT_THROW_MSG(
      // will end up asking for _5, etc.
      cmdstan::get_vec_var_context(init_template_good, 4, 3),
      std::invalid_argument, "Cannot open some of the requested files: [");
  // a bit awkward to test twice, but can't match full string due to OS-paths
  EXPECT_THROW_MSG(cmdstan::get_vec_var_context(init_template_good, 4, 3),
                   std::invalid_argument, "Also failed to find base file");
}

TEST_F(VarContextLoading, multi_template_warning_R_extension) {
  testing::internal::CaptureStderr();
  auto var_context = cmdstan::get_vec_var_context(init_template_R_good, 4, 1);
  std::string output = testing::internal::GetCapturedStderr();
  EXPECT_IN_STRING(
      "This format is deprecated and will not receive new features.", output);
  EXPECT_EQ(4, var_context.size());
  EXPECT_FLOAT_EQ(0.1, var_context[0]->vals_r("theta")[0]);
  EXPECT_FLOAT_EQ(0.2, var_context[1]->vals_r("theta")[0]);
}

TEST_F(VarContextLoading, multi_empty) {
  auto var_context = cmdstan::get_vec_var_context("", 4, 1);
  EXPECT_EQ(4, var_context.size());
  std::vector<std::string> names;
  for (size_t i = 0; i < 4; ++i) {
    var_context[i]->names_r(names);
    EXPECT_EQ(0, names.size());
  }
}

// --------------- multi-chain, comma-separated ---------------

TEST_F(VarContextLoading, multi_comma_good) {
  auto var_context = cmdstan::get_vec_var_context(
      init_good + "," + init_template_good_member, 2, 1);
  EXPECT_EQ(2, var_context.size());
  EXPECT_FLOAT_EQ(0.1, var_context[0]->vals_r("theta")[0]);
  EXPECT_FLOAT_EQ(0.2, var_context[1]->vals_r("theta")[0]);
}

TEST_F(VarContextLoading, multi_comma_wrong_number){
    EXPECT_THROW_MSG(cmdstan::get_vec_var_context(
                         init_good + "," + init_template_good_member, 4, 1),
                     std::invalid_argument,
                     "Number of filenames does not match number of chains: got "
                     "comma-separated list '")}

TEST_F(VarContextLoading, multi_comma_bad_extension) {
  EXPECT_THROW_MSG(
      cmdstan::get_vec_var_context(init_good + "," + init_good + "foo", 2, 1),
      std::invalid_argument,
      "User specified files must end in .json or .R. Found: .jsonfoo");
}

TEST_F(VarContextLoading, multi_comma_missing_file) {
  EXPECT_THROW_MSG(cmdstan::get_vec_var_context(
                       init_good + "," + init_template_missing_member, 2, 3),
                   std::invalid_argument,
                   "Cannot open some of the requested files: ");
}

TEST_F(VarContextLoading, multi_comma_warning_R_extension) {
  testing::internal::CaptureStderr();
  auto var_context = cmdstan::get_vec_var_context(
      init_good + "," + init_template_R_good_member, 2, 1);
  std::string output = testing::internal::GetCapturedStderr();
  EXPECT_IN_STRING(
      "This format is deprecated and will not receive new features.", output);
  EXPECT_EQ(2, var_context.size());
  EXPECT_FLOAT_EQ(0.1, var_context[0]->vals_r("theta")[0]);
  EXPECT_FLOAT_EQ(0.2, var_context[1]->vals_r("theta")[0]);
}
