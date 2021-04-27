#include <test/test-models/proper.hpp>
#include <test/utility.hpp>
#include <cmdstan/return_codes.hpp>
#include <stan/callbacks/stream_writer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/math/policies/error_handling.hpp>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

using cmdstan::test::convert_model_path;
using cmdstan::test::count_matches;
using cmdstan::test::multiple_command_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

TEST(StanUiCommand, countMatches) {
  EXPECT_EQ(-1, count_matches("", ""));
  EXPECT_EQ(-1, count_matches("", "abc"));

  EXPECT_EQ(0, count_matches("abc", ""));
  EXPECT_EQ(0, count_matches("abc", "ab"));
  EXPECT_EQ(0, count_matches("abc", "dab"));
  EXPECT_EQ(0, count_matches("abc", "abde"));

  EXPECT_EQ(0, count_matches("aa", "a"));
  EXPECT_EQ(1, count_matches("aa", "aa"));
  EXPECT_EQ(1, count_matches("aa", "aaa"));
  EXPECT_EQ(2, count_matches("aa", "aaaa"));
}

void test_sample_prints(const std::string &base_cmd) {
  std::string cmd(base_cmd);
  cmd += " --num_samples=100 --num_warmup=100";
  std::string cmd_output = run_command(cmd).output;
  // transformed data
  EXPECT_EQ(1, count_matches("x=", cmd_output));
  // transformed parameters
  EXPECT_TRUE(count_matches("z=", cmd_output) >= 200);
  // model
  EXPECT_TRUE(count_matches("y=", cmd_output) >= 200);
  // generated quantities [only on saved iterations, should be num samples]
  EXPECT_TRUE(count_matches("w=", cmd_output) == 100);
}

void test_optimize_prints(const std::string &base_cmd) {
  std::string cmd(base_cmd);
  std::string cmd_output = run_command(cmd).output;
  // transformed data
  EXPECT_EQ(1, count_matches("x=", cmd_output));
  // transformed parameters
  EXPECT_TRUE(count_matches("z=", cmd_output) >= 1);
  // model
  EXPECT_TRUE(count_matches("y=", cmd_output) >= 1);
  // generated quantities [only on saved iterations, should be num samples]
  EXPECT_TRUE(count_matches("w=", cmd_output) == 1);
}

TEST(StanUiCommand, printReallyPrints) {
  std::vector<std::string> path_vector;
  path_vector.push_back("..");
  path_vector.push_back("src");
  path_vector.push_back("test");
  path_vector.push_back("test-models");
  path_vector.push_back("printer");

  std::string path = "cd test ";
  path += multiple_command_separator();
  path += " ";
  path += convert_model_path(path_vector);

  // SAMPLING
  // static HMC
  // + adapt
  test_sample_prints(path + " sample --hmc --metric unit");
  test_sample_prints(path + " sample --hmc --metric diag");
  test_sample_prints(path + " sample --hmc --metric dense");

  // - adapt
  test_sample_prints(path + " sample --hmc --metric unit --adapt_off");
  test_sample_prints(path + " sample --hmc --metric diag --adapt_off");
  test_sample_prints(path + " sample --hmc --metric dense --adapt_off");

  // NUTS
  // + adapt
  test_sample_prints(path + " sample --nuts --metric unit");
  test_sample_prints(path + " sample --nuts --metric diag");
  test_sample_prints(path + " sample --nuts --metric dense");

  // - adapt
  test_sample_prints(path + " sample --nuts --metric unit --adapt_off");
  test_sample_prints(path + " sample --nuts --metric diag --adapt_off");
  test_sample_prints(path + " sample --nuts --metric dense --adapt_off");

  // OPTIMIZATION
  test_optimize_prints(path + " optimize --newton");
  test_optimize_prints(path + " optimize --bfgs");
}

TEST(StanUiCommand, refresh_zero_ok) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("proper");

  std::string command = convert_model_path(model_path) 
    + " sample --num_samples=10 --num_warmup=10 --init_zero "
    "--refresh=0 --output_file=test/output.csv";
  run_command_output out = run_command(command);
  EXPECT_EQ(int(cmdstan::return_codes::OK), out.err_code);
  EXPECT_EQ(0, count_matches("Iteration:", out.output));
}

TEST(StanUiCommand, refresh_nonzero_ok) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("proper");

  std::string command = convert_model_path(model_path) +
                        " sample --num_samples=10 --num_warmup=10 --init_zero "
                        "--refresh=1 --output_file=test/output.csv";
  run_command_output out = run_command(command);
  EXPECT_EQ(int(cmdstan::return_codes::OK), out.err_code);
  EXPECT_EQ(20, count_matches("Iteration:", out.output));
}

TEST(StanUiCommand, zero_init_value_fail) {
  std::string expected_message = "Rejecting initial value";

  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("value_fail");

  std::string command = convert_model_path(model_path)
                        + " sample --init_zero --output_file test/output.csv";
  run_command_output out = run_command(command);
  EXPECT_EQ(int(cmdstan::return_codes::NOT_OK), out.err_code);

  EXPECT_TRUE(out.header.length() > 0U) << out.output;
  EXPECT_TRUE(out.body.length() > 0U);

  EXPECT_EQ(1, count_matches(expected_message, out.body))
      << "Failed running: " << out.command << std::endl
      << "Expecting to find \"" << expected_message << "\" in" << std::endl
      << out.body;
}

TEST(StanUiCommand, zero_init_domain_fail) {
  std::string expected_message = "Rejecting initial value";

  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("domain_fail");

  std::string command = convert_model_path(model_path)
                        + " sample --init_zero --output_file=test/output.csv";

  run_command_output out = run_command(command);
  EXPECT_EQ(int(cmdstan::return_codes::NOT_OK), out.err_code);

  EXPECT_TRUE(out.header.length() > 0U);
  EXPECT_TRUE(out.body.length() > 0U);

  EXPECT_EQ(1, count_matches(expected_message, out.body))
      << "Failed running: " << out.command << std::endl
      << "Expecting to find \"" << expected_message << "\" in" << std::endl
      << out.body;
}

TEST(StanUiCommand, user_init_value_fail) {
  std::string expected_message = "Rejecting initial value";

  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("value_fail");

  std::vector<std::string> init_path;
  init_path.push_back("src");
  init_path.push_back("test");
  init_path.push_back("test-models");
  init_path.push_back("value_fail.init.R");

  std::string command = convert_model_path(model_path)
                        + " sample --init_file=" + convert_model_path(init_path)
                        + " --output_file=test/output.csv";

  run_command_output out = run_command(command);
  EXPECT_EQ(int(cmdstan::return_codes::NOT_OK), out.err_code);

  EXPECT_TRUE(out.header.length() > 0U);
  EXPECT_TRUE(out.body.length() > 0U);

  EXPECT_EQ(1, count_matches(expected_message, out.body))
      << "Failed running: " << out.command << std::endl
      << "Expecting to find \"" << expected_message << "\" in" << std::endl
      << out.body;
}

TEST(StanUiCommand, user_init_domain_fail) {
  std::string expected_message = "Rejecting initial value";

  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("domain_fail");

  std::vector<std::string> init_path;
  init_path.push_back("src");
  init_path.push_back("test");
  init_path.push_back("test-models");
  init_path.push_back("domain_fail.init.R");

  std::string command = convert_model_path(model_path)
                        + " sample --init_file=" + convert_model_path(init_path)
                        + " --output_file=test/output.csv";

  run_command_output out = run_command(command);
  EXPECT_EQ(int(cmdstan::return_codes::NOT_OK), out.err_code);

  EXPECT_TRUE(out.header.length() > 0U);
  EXPECT_TRUE(out.body.length() > 0U);

  EXPECT_EQ(1, count_matches(expected_message, out.body))
      << "Failed running: " << out.command << std::endl
      << "Expecting to find \"" << expected_message << "\" in" << std::endl
      << out.body;
}

TEST(StanUiCommand, CheckCommand_no_args) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("domain_fail");  // can use any model here

  std::string command = convert_model_path(model_path);
  run_command_output out = run_command(command);
  EXPECT_EQ(int(cmdstan::return_codes::NOT_OK), out.err_code);
}

TEST(StanUiCommand, CheckCommand_help) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("domain_fail");  // can use any model here

  std::string command = convert_model_path(model_path) + " --help";

  run_command_output out = run_command(command);
  EXPECT_EQ(int(cmdstan::return_codes::OK), out.err_code);
}

TEST(StanUiCommand, CheckCommand_unrecognized_argument) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("domain_fail");  // can use any model here

  std::string command = convert_model_path(model_path) + " foo";

  run_command_output out = run_command(command);
  EXPECT_EQ(int(cmdstan::return_codes::NOT_OK), out.err_code);
}

TEST(StanUiCommand, timing_info) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("proper");

  std::string command = convert_model_path(model_path) +
                        " sample --num_samples=10 --num_warmup=10 --init_zero "
                        "--refresh=0 --output_file=test/output.csv";
  run_command_output out = run_command(command);
  EXPECT_EQ(int(cmdstan::return_codes::OK), out.err_code);

  std::fstream output_csv_stream("test/output.csv");
  std::stringstream output_sstream;
  output_sstream << output_csv_stream.rdbuf();
  output_csv_stream.close();
  std::string output = output_sstream.str();

  EXPECT_EQ(1, count_matches("#  Elapsed Time:", output));
  EXPECT_EQ(1, count_matches(" seconds (Warm-up)", output));
  EXPECT_EQ(1, count_matches(" seconds (Sampling)", output));
  EXPECT_EQ(1, count_matches(" seconds (Total)", output));
}

TEST(StanUiCommand, run_info) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("proper");

  std::string command = convert_model_path(model_path) +
                        " sample --num_samples=10 --num_warmup=10 --init_zero "
                        "--refresh=0 --output_file=test/output.csv";
  run_command_output out = run_command(command);
  EXPECT_EQ(int(cmdstan::return_codes::OK), out.err_code);

  std::fstream output_csv_stream("test/output.csv");
  std::stringstream output_sstream;
  output_sstream << output_csv_stream.rdbuf();
  output_csv_stream.close();
  std::string output = output_sstream.str();

  EXPECT_EQ(1, count_matches("# method = sample", output));
  EXPECT_EQ(1, count_matches(" num_samples = 10", output));
  EXPECT_EQ(1, count_matches(" num_warmup = 10", output));
  EXPECT_EQ(1, count_matches(" init = 0", output));
}

TEST(StanUiCommand, random_seed_default) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("transformed_data_rng_test");

  std::string command
      = convert_model_path(model_path)
        + " sample --num_samples=10 --num_warmup=10 --init_zero "
        + " --data_file=src/test/test-models/transformed_data_rng_test.init.R"
        + " --refresh=0 --output_file=test/output.csv";
  std::string cmd_output = run_command(command).output;
  EXPECT_EQ(1, count_matches("y values:", cmd_output));
  std::vector<std::string> lines;
  split(lines, cmd_output, boost::is_any_of("\n"));
  std::string random1;
  for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end();
       ++it) {
    if (boost::starts_with(*it, "y values:")) {
      random1.assign(*it, 9, std::string::npos);
      EXPECT_EQ(1, count_matches("[", random1));
      EXPECT_EQ(1, count_matches("]", random1));
      break;
    }
  }
  cmd_output = run_command(command).output;
  EXPECT_EQ(1, count_matches("y values:", cmd_output));
  split(lines, cmd_output, boost::is_any_of("\n"));
  std::string random2;
  for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end();
       ++it) {
    if (boost::starts_with(*it, "y values:")) {
      random2.assign(*it, 9, std::string::npos);
      EXPECT_EQ(1, count_matches("[", random2));
      EXPECT_EQ(1, count_matches("]", random2));
      break;
    }
  }
  EXPECT_NE(random1, random2);
}

TEST(StanUiCommand, random_seed_specified_same) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("transformed_data_rng_test");

  std::string command
      = convert_model_path(model_path)
        + " sample --num_samples=10 --num_warmup=10 --init_zero "
        + " --seed=12345 "
        + " --data_file=src/test/test-models/transformed_data_rng_test.init.R"
        + " --refresh=0 --output_file=test/output.csv";
  std::string cmd_output = run_command(command).output;
  EXPECT_EQ(1, count_matches("y values:", cmd_output));
  std::vector<std::string> lines;
  split(lines, cmd_output, boost::is_any_of("\n"));
  std::string random1;
  for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end();
       ++it) {
    if (boost::starts_with(*it, "y values:")) {
      random1.assign(*it, 9, std::string::npos);
      EXPECT_EQ(1, count_matches("[", random1));
      EXPECT_EQ(1, count_matches("]", random1));
      break;
    }
  }
  cmd_output = run_command(command).output;
  EXPECT_EQ(1, count_matches("y values:", cmd_output));
  split(lines, cmd_output, boost::is_any_of("\n"));
  std::string random2;
  for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end();
       ++it) {
    if (boost::starts_with(*it, "y values:")) {
      random2.assign(*it, 9, std::string::npos);
      EXPECT_EQ(1, count_matches("[", random2));
      EXPECT_EQ(1, count_matches("]", random2));
      break;
    }
  }
  EXPECT_EQ(random1, random2);
}

TEST(StanUiCommand, random_seed_specified_different) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("transformed_data_rng_test");

  std::string command
      = convert_model_path(model_path)
        + " sample --num_samples=10 --num_warmup=10 --init_zero "
        + " --seed=12345 "
        + " --data_file=src/test/test-models/transformed_data_rng_test.init.R"
        + " --refresh=0 --output_file=test/output.csv";
  std::string cmd_output = run_command(command).output;
  EXPECT_EQ(1, count_matches("y values:", cmd_output));
  std::vector<std::string> lines;
  split(lines, cmd_output, boost::is_any_of("\n"));
  std::string random1;
  for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end();
       ++it) {
    if (boost::starts_with(*it, "y values:")) {
      random1.assign(*it, 9, std::string::npos);
      EXPECT_EQ(1, count_matches("[", random1));
      EXPECT_EQ(1, count_matches("]", random1));
      break;
    }
  }
  command
      = convert_model_path(model_path)
        + " sample --num_samples=10 --num_warmup=10 --init_radius=0 "
        + " --seed=45678 "
        + " --data_file=src/test/test-models/transformed_data_rng_test.init.R"
        + " --refresh=0 --output_file=test/output.csv";
  cmd_output = run_command(command).output;
  EXPECT_EQ(1, count_matches("y values:", cmd_output));
  split(lines, cmd_output, boost::is_any_of("\n"));
  std::string random2;
  for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end();
       ++it) {
    if (boost::starts_with(*it, "y values:")) {
      random2.assign(*it, 9, std::string::npos);
      EXPECT_EQ(1, count_matches("[", random2));
      EXPECT_EQ(1, count_matches("]", random2));
      break;
    }
  }
  EXPECT_NE(random1, random2);
}

TEST(StanUiCommand, random_seed_fail_1) {
  std::string expected_message = "--seed: Number less or equal to 0:";

  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("transformed_data_rng_test");

  std::string command
      = convert_model_path(model_path)
        + " sample --num_samples=10 --num_warmup=10 --init_zero "
        + " --seed=-2 "
        + " --data_file=src/test/test-models/transformed_data_rng_test.init.R"
        + " --refresh=0 --output_file=test/output.csv";
  std::string cmd_output = run_command(command).output;
  run_command_output out = run_command(command);
  EXPECT_EQ(1, count_matches(expected_message, out.body))
      << "expected: \"" << expected_message << "\"" << std::endl
      << "found:    \"" << out.body << "\"" << std::endl
      << out;
}

TEST(StanUiCommand, json_input) {
  std::vector<std::string> model_path;
  model_path.push_back("src");
  model_path.push_back("test");
  model_path.push_back("test-models");
  model_path.push_back("ndim_array");

  std::string command
      = convert_model_path(model_path) + " sample " + " --fixed_param"
        + " --seed=12345 "
        + " --data_file=src/test/test-models/ndim_array.data.json"
        + " --refresh=0 --output_file=test/output.csv";
  std::string cmd_output = run_command(command).output;

  EXPECT_EQ(
      1, count_matches("d1_1: [[0,1,2,3],[4,5,6,7],[8,9,10,11]]", cmd_output));
  EXPECT_EQ(1,
            count_matches("d1_2: [[12,13,14,15],[16,17,18,19],[20,21,22,23]]",
                          cmd_output));
}

//
struct dummy_stepsize_adaptation {
  void set_mu(const double) {}
  void set_delta(const double) {}
  void set_gamma(const double) {}
  void set_kappa(const double) {}
  void set_t0(const double) {}
};

struct dummy_z {
  Eigen::VectorXd q;
};

template <class ExceptionType>
struct sampler {
  dummy_stepsize_adaptation _stepsize_adaptation;
  dummy_z _z;

  double get_nominal_stepsize() { return 0; }
  dummy_stepsize_adaptation get_stepsize_adaptation() {
    return _stepsize_adaptation;
  }

  void engage_adaptation() {}
  dummy_z z() { return _z; }

  void init_stepsize(stan::callbacks::writer &info_writer,
                     stan::callbacks::writer &error_writer) {
    throw ExceptionType("throwing exception");
  }
};
