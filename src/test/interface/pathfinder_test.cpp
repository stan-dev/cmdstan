#include <test/utility.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <gtest/gtest.h>

using cmdstan::test::convert_model_path;
using cmdstan::test::parse_sample;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    multi_normal_model = {"src", "test", "test-models", "multi_normal_model"};
    default_file_path = {"src", "test", "test-models", "output.csv"};
    dev_null_path = {"/dev", "null"};
    simplex_model = {"src", "test", "test-models", "simplex_model"};

  }
  std::vector<std::string> default_file_path;
  std::vector<std::string> dev_null_path;
  std::vector<std::string> simple_jacobian_model;
  std::vector<std::string> multi_normal_model;
  std::vector<std::string> simplex_model;
};

TEST_F(CmdStan, pathfinder_good) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=pathfinder";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);

  ss.str(std::string());
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=pathfinder";
  out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
}

// TEST_F(CmdStan, pathfinder_single_good) {
// }

// TEST_F(CmdStan, pathfinder_multi_good) {
// }

// TEST_F(CmdStan, pathfinder_diag_json) {
// }

