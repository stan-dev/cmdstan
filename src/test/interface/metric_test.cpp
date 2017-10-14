#include <stan/services/error_codes.hpp>
#include <cmdstan/command.hpp>
#include <gtest/gtest.h>
#include <string>
#include <test/utility.hpp>
#include <stdexcept>
#include <boost/math/policies/error_handling.hpp>
#include <stan/callbacks/stream_writer.hpp>

using cmdstan::test::convert_model_path;
using cmdstan::test::count_matches;
using cmdstan::test::multiple_command_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

TEST(StanUiCommand, metric_file_test) {
  std::vector<std::string> models = { "proper", "test_model" };
  std::vector<std::string> engines = { "static", "nuts" };
  std::vector<std::string> metrics = { "diag_e", "dense_e" };
  std::vector<std::string> adapts = { "0", "1" };

  std::map<std::string, std::map<std::string, std::string> > expected_output = {
    { "proper", {
        { "diag_e", "# Diagonal elements of inverse mass matrix:\n# 1.5" },
        { "dense_e", "# Elements of inverse mass matrix:\n# 1.5" }
      }
    },
    { "test_model", {
        { "diag_e", "# Diagonal elements of inverse mass matrix:\n# 1.5, 1.8" },
        { "dense_e", "# Elements of inverse mass matrix:\n# 1.5, 1\n# 1, 1.5" }
      }
    }
  };

  for (auto model : models) {
    for (auto engine : engines) {
      for (auto metric : metrics) {
        for (auto adapt : adapts) {
          std::vector<std::string> model_path;
          model_path.push_back("src");
          model_path.push_back("test");
          model_path.push_back("test-models");
          model_path.push_back(model);

          std::vector<std::string> metric_file_path;
          metric_file_path.push_back("src");
          metric_file_path.push_back("test");
          metric_file_path.push_back("test-models");
          metric_file_path.push_back(model + "." + metric + "_metric.dat.R");

          std::string command = convert_model_path(model_path) +
            " method=sample num_samples=1 num_warmup=1 adapt engaged=" + adapt +
            " algorithm=hmc engine=" + engine + " metric=" + metric +
            " metric_file=" + convert_model_path(metric_file_path) +
            " output file=test/output.csv";

          run_command_output out = run_command(command);
          EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code);

          std::fstream output_csv_stream("test/output.csv");
          std::stringstream output_sstream;
          output_sstream << output_csv_stream.rdbuf();
          output_csv_stream.close();
          std::string output = output_sstream.str();

          EXPECT_EQ(1, count_matches(expected_output[model][metric], output));
        }
      }
    }
  }
}
