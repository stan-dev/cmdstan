#include <test/utility.hpp>
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <stdexcept>
#include <stan/io/stan_csv_reader.hpp>

using cmdstan::test::convert_model_path;
using cmdstan::test::multiple_command_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

class CmdStan : public testing::Test {
 public:
  void SetUp() {
    bern_gq_model = {"src", "test", "test-models", "bern_gq_model"};
    bern_extra_model = {"src", "test", "test-models", "bern_extra_model"};
    bern_data = {"src", "test", "test-models", "bern.data.json"};
    bern_fitted_params
        = {"src", "test", "test-models", "bern_fitted_params.csv"};
    bern_fitted_params_multi
        = {"src", "test", "test-models", "bern_params_multi.csv"};
    bern_fitted_params_warmup
        = {"src", "test", "test-models", "bern_fitted_params_warmup.csv"};
    bern_optimized_params
        = {"src", "test", "test-models", "bern_optimized_params.csv"};
    bern_variational_params
        = {"src", "test", "test-models", "bern_optimized_params.csv"};
    bern_fitted_params_thin
        = {"src", "test", "test-models", "bern_fitted_params_thin.csv"};
    default_file_path = {"src", "test", "test-models", "output.csv"};
    dev_null_path = {"/dev", "null"};
    gq_non_scalar_model = {"src", "test", "test-models", "gq_non_scalar"};
    gq_non_scalar_fitted_params
        = {"src", "test", "test-models", "gq_non_scalar_fitted_params.csv"};
    test_model = {"src", "test", "test-models", "test_model"};
  }
  std::vector<std::string> bern_gq_model;
  std::vector<std::string> bern_extra_model;
  std::vector<std::string> bern_data;
  std::vector<std::string> bern_fitted_params;
  std::vector<std::string> bern_fitted_params_multi;
  std::vector<std::string> bern_fitted_params_warmup;
  std::vector<std::string> bern_optimized_params;
  std::vector<std::string> bern_variational_params;
  std::vector<std::string> bern_fitted_params_thin;
  std::vector<std::string> default_file_path;
  std::vector<std::string> dev_null_path;
  std::vector<std::string> gq_non_scalar_model;
  std::vector<std::string> gq_non_scalar_fitted_params;
  std::vector<std::string> test_model;
};

TEST_F(CmdStan, generate_quantities_good) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_good_multi) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params_multi) << " num_chains=4";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_same_in_out_multi) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(bern_fitted_params_multi)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params_multi) << " num_chains=4";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_same_in_out_multi_path_diff) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " data file=" << convert_model_path(bern_data) << " output file="
     << std::string("./") + convert_model_path(bern_fitted_params_multi)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params_multi) << " num_chains=4";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_non_scalar_good) {
  std::stringstream ss;
  ss << convert_model_path(gq_non_scalar_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(gq_non_scalar_fitted_params);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_no_data_arg) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_no_fitted_params_arg) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_missing_fitted_params) {
  std::stringstream ss;
  ss << convert_model_path(bern_extra_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_wrong_csv) {
  std::stringstream ss;
  ss << convert_model_path(test_model)
     << " output file=" << convert_model_path(dev_null_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params) << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_csv_conflict) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(default_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(default_file_path);  // << " 2>&1";
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_TRUE(out.hasError);
}

TEST_F(CmdStan, generate_quantities_warmup) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(default_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params_warmup);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);

  std::stringstream msg;
  std::string fp_path = convert_model_path(bern_fitted_params_warmup);
  std::string gq_output_path = convert_model_path(default_file_path);

  std::ifstream fp_stream(fp_path.c_str());
  stan::io::stan_csv fitted_params;
  stan::io::stan_csv_reader::read_metadata(fp_stream, fitted_params.metadata);
  stan::io::stan_csv_reader::read_header(fp_stream, fitted_params.header,
                                         false);
  stan::io::stan_csv_reader::read_samples(fp_stream, fitted_params.samples,
                                          fitted_params.timing);
  fp_stream.close();

  std::ifstream gq_stream(gq_output_path.c_str());
  stan::io::stan_csv gq_output;
  stan::io::stan_csv_reader::read_samples(gq_stream, gq_output.samples,
                                          gq_output.timing);
  stan::io::stan_csv_reader::read_metadata(gq_stream, gq_output.metadata);
  stan::io::stan_csv_reader::read_header(gq_stream, gq_output.header, false);
  stan::io::stan_csv_reader::read_samples(gq_stream, gq_output.samples,
                                          gq_output.timing);
  gq_stream.close();

  ASSERT_EQ(fitted_params.samples.rows(), gq_output.samples.rows());
}

TEST_F(CmdStan, generate_quantities_after_optimization) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(default_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_optimized_params);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);

  std::stringstream msg;
  std::string fp_path = convert_model_path(bern_optimized_params);
  std::string gq_output_path = convert_model_path(default_file_path);

  std::ifstream fp_stream(fp_path.c_str());
  stan::io::stan_csv fitted_params;
  stan::io::stan_csv_reader::read_metadata(fp_stream, fitted_params.metadata);
  stan::io::stan_csv_reader::read_header(fp_stream, fitted_params.header,
                                         false);
  stan::io::stan_csv_reader::read_samples(fp_stream, fitted_params.samples,
                                          fitted_params.timing);
  fp_stream.close();

  std::ifstream gq_stream(gq_output_path.c_str());
  stan::io::stan_csv gq_output;
  stan::io::stan_csv_reader::read_samples(gq_stream, gq_output.samples,
                                          gq_output.timing);
  stan::io::stan_csv_reader::read_metadata(gq_stream, gq_output.metadata);
  stan::io::stan_csv_reader::read_header(gq_stream, gq_output.header, false);
  stan::io::stan_csv_reader::read_samples(gq_stream, gq_output.samples,
                                          gq_output.timing);
  gq_stream.close();

  ASSERT_EQ(fitted_params.samples.rows(), gq_output.samples.rows());
}

TEST_F(CmdStan, generate_quantities_after_vb) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(default_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_variational_params);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);

  std::stringstream msg;
  std::string fp_path = convert_model_path(bern_variational_params);
  std::string gq_output_path = convert_model_path(default_file_path);

  std::ifstream fp_stream(fp_path.c_str());
  stan::io::stan_csv fitted_params;
  stan::io::stan_csv_reader::read_metadata(fp_stream, fitted_params.metadata);
  stan::io::stan_csv_reader::read_header(fp_stream, fitted_params.header,
                                         false);
  stan::io::stan_csv_reader::read_samples(fp_stream, fitted_params.samples,
                                          fitted_params.timing);
  fp_stream.close();

  std::ifstream gq_stream(gq_output_path.c_str());
  stan::io::stan_csv gq_output;
  stan::io::stan_csv_reader::read_samples(gq_stream, gq_output.samples,
                                          gq_output.timing);
  stan::io::stan_csv_reader::read_metadata(gq_stream, gq_output.metadata);
  stan::io::stan_csv_reader::read_header(gq_stream, gq_output.header, false);
  stan::io::stan_csv_reader::read_samples(gq_stream, gq_output.samples,
                                          gq_output.timing);
  gq_stream.close();

  ASSERT_EQ(fitted_params.samples.rows(), gq_output.samples.rows());
}

TEST_F(CmdStan, generate_quantities_thin) {
  std::stringstream ss;
  ss << convert_model_path(bern_gq_model)
     << " data file=" << convert_model_path(bern_data)
     << " output file=" << convert_model_path(default_file_path)
     << " method=generate_quantities fitted_params="
     << convert_model_path(bern_fitted_params_thin);
  std::string cmd = ss.str();
  run_command_output out = run_command(cmd);
  ASSERT_FALSE(out.hasError);

  std::stringstream msg;
  std::string fp_path = convert_model_path(bern_fitted_params_thin);
  std::string gq_output_path = convert_model_path(default_file_path);

  std::ifstream fp_stream(fp_path.c_str());
  stan::io::stan_csv fitted_params;
  stan::io::stan_csv_reader::read_metadata(fp_stream, fitted_params.metadata);
  stan::io::stan_csv_reader::read_header(fp_stream, fitted_params.header,
                                         false);
  stan::io::stan_csv_reader::read_samples(fp_stream, fitted_params.samples,
                                          fitted_params.timing);
  fp_stream.close();

  std::ifstream gq_stream(gq_output_path.c_str());
  stan::io::stan_csv gq_output;
  stan::io::stan_csv_reader::read_samples(gq_stream, gq_output.samples,
                                          gq_output.timing);
  stan::io::stan_csv_reader::read_metadata(gq_stream, gq_output.metadata);
  stan::io::stan_csv_reader::read_header(gq_stream, gq_output.header, false);
  stan::io::stan_csv_reader::read_samples(gq_stream, gq_output.samples,
                                          gq_output.timing);
  gq_stream.close();

  ASSERT_EQ(fitted_params.samples.rows(), gq_output.samples.rows());
}
