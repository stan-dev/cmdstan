#include <cmdstan/return_codes.hpp>
#include <cmdstan/stansummary_helper.hpp>
#include <stan/mcmc/chainset.hpp>
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
                              comma-separated numbers from (0.1,99.9), inclusive.
                              Default is 5,50,95.
  -s, --sig_figs [n]          Significant figures reported. Default is 2.
                              Must be an integer from (1, 18), inclusive.
  -i, --include_param [name]  Include the named parameter in the summary output.
                              By default, all parameters in the file are summarized,
                              passing this argument one or more times will filter
                              the output down to just the requested arguments.
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
  std::vector<std::string> requested_params_vec;

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
  app.add_option("--include_param,-i", requested_params_vec,
                 "Include the named parameter in the output. By default all "
                 "are included.",
                 true)
    ->transform([](auto str) {
        // allow both 'theta.1' and 'theta[1]' style.
        std::string token(str);
        stan::io::prettify_stan_csv_name(token);
        return token;
      })
    ->take_all();
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
  Eigen::VectorXd probs;
  boost::algorithm::trim(percentiles_spec);
  if (!percentiles_spec.empty()) {
    boost::algorithm::split(percentiles, percentiles_spec,
                            boost::is_any_of(", "), boost::token_compress_on);
    try {
      probs = percentiles_to_probs(percentiles);
    } catch (const std::invalid_argument &e) {
      std::cout << "Option --percentiles " << percentiles_spec << ": "
                << e.what() << std::endl;
      return return_codes::NOT_OK;
    }
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

  std::vector<stan::io::stan_csv> csv_files;
  Eigen::VectorXd warmup_times(filenames.size());
  Eigen::VectorXd sampling_times(filenames.size());
  Eigen::VectorXi thin(filenames.size());
  for (int i = 0; i < filenames.size(); ++i) {
    std::ifstream infile;
    std::stringstream out;
    stan::io::stan_csv sample;
    infile.open(filenames[i].c_str());
    try {
      sample = stan::io::stan_csv_reader::parse(infile, &out);
      // csv_reader warnings are errors - fail fast.
      if (!out.str().empty()) {
	throw std::invalid_argument(out.str());
      }
      csv_files.push_back(sample);
      warmup_times(i) = sample.timing.warmup;
      sampling_times(i) = sample.timing.sampling;
      thin(i) = sample.metadata.thin;
    } catch (const std::invalid_argument &e) {
      std::cout << "Cannot parse input csv file: " << filenames[i]
		<< e.what() << "." << std::endl;
      return return_codes::NOT_OK;
    }
  }
  stan::io::stan_csv_metadata metadata = csv_files[0].metadata;
  std::vector<std::string> param_names = csv_files[0].header;
  if (requested_params_vec.size() > 0) {
    std::set<std::string> requested_params(requested_params_vec.begin(), requested_params_vec.end());
    std::set<std::string> check_requested_params(requested_params_vec.begin(), requested_params_vec.end());
    for (size_t i = 0; i < param_names.size(); ++i) {
      check_requested_params.erase(param_names[i]);
    }
    if (check_requested_params.size() == 0) {
      param_names.clear();
      std::copy(requested_params.begin(), requested_params.end(), std::back_inserter(param_names));
    } else {
      std::cout << "--include_param: Unrecognized parameter(s): ";
      for (auto param : requested_params) {
	std::cout << "'" << param << "' ";
      }
      std::cout << std::endl;
      return return_codes::NOT_OK;
    }
  }
  size_t num_params = param_names.size();
  size_t max_name_length = 0;
  for (size_t i = 0; i < num_params; ++i) {
    max_name_length = std::max(param_names[i].size(), max_name_length);
  }
  try {
    std::vector<std::string> header = get_header(percentiles);
    stan::mcmc::chainset<> chains(csv_files);

    Eigen::MatrixXd param_stats(num_params, header.size());
    get_stats(chains, probs, param_names, param_stats);

    // Console output formatting
    Eigen::VectorXi column_sig_figs(header.size());
    Eigen::Matrix<std::ios_base::fmtflags, Eigen::Dynamic, 1> column_formats(header.size());

    Eigen::VectorXi column_widths(header.size());
    column_widths = calculate_column_widths(param_stats, header, sig_figs,
					     column_formats);

    // Print to console
    write_timing(chains, metadata, warmup_times, sampling_times, thin, "",
		 &std::cout);
    std::cout << std::endl;
    write_header(header, column_widths, max_name_length, false, &std::cout);
    std::cout << std::endl;
    write_stats(param_names, param_stats, column_widths, column_formats,
		max_name_length, sig_figs, false, &std::cout);
    std::cout << std::endl;
    write_sampler_info(metadata, "", &std::cout);

    if (app.count("--autocorr")) {
      autocorrelation(chains, metadata, autocorr_idx, max_name_length);
      std::cout << std::endl;
    }

    // Write to csv file (optional)
    if (app.count("--csv_filename")) {
      std::ofstream csv_file(csv_filename.c_str(), std::ios_base::app);
      csv_file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
      csv_file << std::setprecision(app.count("--sig_figs") ? sig_figs : 6);

      write_header(header, column_widths, max_name_length, true, &csv_file);
      write_stats(param_names, param_stats, column_widths, column_formats,
		  max_name_length, sig_figs, true, &csv_file);
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
