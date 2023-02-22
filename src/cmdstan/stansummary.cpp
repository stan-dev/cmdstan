#include <cmdstan/return_codes.hpp>
#include <cmdstan/stansummary_helper.hpp>
#include <stan/mcmc/chains.hpp>
#include <stan/io/ends_with.hpp>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <CLI11/CLI11.hpp>

using cmdstan::return_codes;

/**
 * Compute summary statistics over HMC sampler output
 * read in from stan_csv files.
 * Command line options handled by CLI11 lib.
 *
 * @param argc Number of arguments
 * @param argv Arguments
 *
 * @return OK for success,
 *         non-zero otherwise
 */
int main(int argc, const char *argv[]) {
  std::string usage = R"(Usage: stansummary [OPTIONS] stan_csv_file(s)
Report statistics for one or more Stan csv files from a HMC sampler run.
Example:  stansummary model_chain_1.csv model_chain_2.csv
Options:
  -a, --autocorr [n]          Display the chain autocorrelation for the n-th
                              input file, in addition to statistics.
  -c, --csv_filename [file]   Write statistics to a csv file.
  -h, --help                  Produce help message, then exit.
  -p, --percentiles [values]  Percentiles to report as ordered set of
                              comma-separated integers from (1,99), inclusive.
                              Default is 5,50,95.
  -s, --sig_figs [n]          Significant figures reported. Default is 2.
                              Must be an integer from (1, 18), inclusive.
)";
  if (argc < 2) {
    std::cout << usage << std::endl;
    return return_codes::NOT_OK;
  }

  // Command-line arguments
  int sig_figs = 2;
  int autocorr_idx;
  std::string csv_filename;
  std::string percentiles_spec = "5,50,95";
  std::vector<std::string> filenames;

  CLI::App app{"Allowed options"};
  app.add_option("--sig_figs,-s", sig_figs, "Significant figures, default 2.",
                 true)
      ->check(CLI::Range(1, 18));
  app.add_option("--autocorr,-a", autocorr_idx,
                 "Display the chain autocorrelation.", true)
      ->check(CLI::PositiveNumber);
  app.add_option("--csv_filename,-c", csv_filename,
                 "Write statistics to a csv.", true)
      ->check(CLI::NonexistentPath);
  app.add_option("--percentiles,-p", percentiles_spec, "Percentiles to report.",
                 true);
  app.add_option("input_files", filenames, "Sampler csv files.", true)
      ->required()
      ->check(CLI::ExistingFile);

  try {
    CLI11_PARSE(app, argc, argv);
  } catch (const CLI::ParseError &e) {
    std::cout << e.get_exit_code();
    return app.exit(e);
  }

  // Check options semantic consistency
  if (app.count("--autocorr") && autocorr_idx > filenames.size()) {
    std::cout << "Option --autocorr: " << autocorr_idx
              << " not a valid chain id." << std::endl;
    return return_codes::NOT_OK;
  }
  std::vector<std::string> percentiles;
  boost::algorithm::trim(percentiles_spec);
  boost::algorithm::split(percentiles, percentiles_spec, boost::is_any_of(", "),
                          boost::token_compress_on);
  Eigen::VectorXd probs;
  try {
    probs = percentiles_to_probs(percentiles);
  } catch (const std::invalid_argument &e) {
    std::cout << "Option --percentiles " << percentiles_spec << ": " << e.what()
              << std::endl;
    return return_codes::NOT_OK;
  }
  if (app.count("--csv_filename")) {
    if (FILE *file = fopen(csv_filename.c_str(), "w")) {
      fclose(file);
    } else {
      std::cout << "Cannot save to csv_filename: " << csv_filename << "."
                << std::endl;
      return return_codes::NOT_OK;
    }
  }
  for (int i = 0; i < filenames.size(); ++i) {
    std::ifstream infile;
    infile.open(filenames[i].c_str());
    if (infile.good()) {
      infile.close();
    } else {
      std::cout << "Cannot read input csv file: " << filenames[i] << "."
                << std::endl;
      return return_codes::NOT_OK;
    }
  }

  try {
    // Parse csv files into sample, metadata
    stan::io::stan_csv_metadata metadata;
    Eigen::VectorXd warmup_times(filenames.size());
    Eigen::VectorXd sampling_times(filenames.size());
    Eigen::VectorXi thin(filenames.size());

    // check for stan csv file parse errors written to output stream
    std::stringstream cout_ss;
    stan::mcmc::chains<> chains = parse_csv_files(
        filenames, metadata, warmup_times, sampling_times, thin, &std::cout);

    // Get column headers for sampler, model params
    size_t max_name_length = 0;
    size_t num_sampler_params = -1;  // don't count name 'lp__'
    for (int i = 0; i < chains.num_params(); ++i) {
      if (chains.param_name(i).length() > max_name_length)
        max_name_length = chains.param_name(i).length();
      if (stan::io::ends_with("__", chains.param_name(i)))
        num_sampler_params++;
    }
    size_t model_params_offset = num_sampler_params + 1;
    size_t num_model_params = chains.num_params() - model_params_offset;

    std::vector<std::string> header = get_header(percentiles);

    // Compute statistics for sampler and model params
    Eigen::MatrixXd lp_param(1, header.size());
    Eigen::MatrixXd sampler_params(num_sampler_params, header.size());
    Eigen::MatrixXd model_params(num_model_params, header.size());

    get_stats(chains, warmup_times, sampling_times, probs, 0, lp_param);
    get_stats(chains, warmup_times, sampling_times, probs, 1, sampler_params);
    get_stats(chains, warmup_times, sampling_times, probs, model_params_offset,
              model_params);

    // Console output formatting
    Eigen::VectorXi column_sig_figs(header.size());
    Eigen::Matrix<std::ios_base::fmtflags, Eigen::Dynamic, 1> sampler_formats(
        header.size());
    Eigen::VectorXi sampler_widths(header.size());
    sampler_widths = calculate_column_widths(sampler_params, header, sig_figs,
                                             sampler_formats);

    Eigen::Matrix<std::ios_base::fmtflags, Eigen::Dynamic, 1> model_formats(
        header.size());
    Eigen::VectorXi model_widths(header.size());
    model_widths = calculate_column_widths(model_params, header, sig_figs,
                                           model_formats);

    Eigen::VectorXi column_widths(header.size());
    for (size_t i = 0; i < header.size(); ++i)
      column_widths[i] = sampler_widths[i] > model_widths[i] ? sampler_widths[i]
                                                             : model_widths[i];

    // Print to console
    write_timing(chains, metadata, warmup_times, sampling_times, thin, "",
                 &std::cout);
    std::cout << std::endl;

    write_header(header, column_widths, max_name_length, false, &std::cout);
    std::cout << std::endl;
    write_params(chains, lp_param, column_widths, model_formats,
                 max_name_length, sig_figs, 0, false, &std::cout);
    write_params(chains, sampler_params, column_widths, sampler_formats,
                 max_name_length, sig_figs, 1, false, &std::cout);
    std::cout << std::endl;
    write_params(chains, model_params, column_widths, model_formats,
                 max_name_length, sig_figs, model_params_offset, false,
                 &std::cout);
    std::cout << std::endl;
    write_sampler_info(metadata, "", &std::cout);

    if (app.count("--autocorr")) {
      autocorrelation(chains, metadata, autocorr_idx, max_name_length);
      std::cout << std::endl;
    }

    // Write to csv file (optional)
    if (app.count("--csv_filename")) {
      std::ofstream csv_file(csv_filename.c_str(), std::ios_base::app);
      csv_file << std::setprecision(app.count("--sig_figs") ? sig_figs : 6);

      write_header(header, column_widths, max_name_length, true, &csv_file);
      write_params(chains, lp_param, column_widths, model_formats,
                   max_name_length, sig_figs, 0, true, &csv_file);
      write_params(chains, sampler_params, column_widths, sampler_formats,
                   max_name_length, sig_figs, 1, true, &csv_file);
      write_params(chains, model_params, column_widths, model_formats,
                   max_name_length, sig_figs, model_params_offset, true,
                   &csv_file);

      write_timing(chains, metadata, warmup_times, sampling_times, thin, "# ",
                   &csv_file);
      write_sampler_info(metadata, "# ", &csv_file);
      csv_file.close();
    }
  } catch (const std::invalid_argument &e) {
    std::cout << "Error during processing. " << e.what() << std::endl;
    return return_codes::NOT_OK;
  }

  return return_codes::OK;
}
