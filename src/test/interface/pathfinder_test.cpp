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
    test_tmp_out_csv = {"test", "tmp_pf.csv"};
    test_tmp_out_json = {"test", "tmp_pf.json"};
    simplex_model = {"src", "test", "test-models", "simplex_model"};
  }
  std::vector<std::string> default_file_path;
  std::vector<std::string> dev_null_path;
  std::vector<std::string> test_tmp_out_csv;
  std::vector<std::string> test_tmp_out_json;
  std::vector<std::string> simple_jacobian_model;
  std::vector<std::string> multi_normal_model;
  std::vector<std::string> simplex_model;
};

TEST_F(CmdStan, pathfinder_good) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(test_tmp_out_csv)
     << " method=pathfinder";
  run_command_output out = run_command(ss.str());
  // check individual path csv files
  // check pathfinder csv file
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, pathfinder_single_good) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(test_tmp_out_csv)
     << " method=pathfinder"
     << " num_paths=1";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, pathfinder_multi_good) {
  std::stringstream ss;
  ss << convert_model_path(multi_normal_model)
     << " output file=" << convert_model_path(test_tmp_out_csv)
     << " method=pathfinder"
     << " num_paths=8";
  run_command_output out = run_command(ss.str());
  ASSERT_FALSE(out.hasError);
}

// TEST_F(CmdStan, pathfinder_diagnostic_json) {
// }
