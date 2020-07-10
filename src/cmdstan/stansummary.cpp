#include <cmdstan/stansummary_helper.hpp>
#include <stan/mcmc/chains.hpp>
#include <stan/io/ends_with.hpp>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <vector>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

/**
 * Compute summary statistics over NUTS-HMC sampler output
 * read in from stan_csv files.
 *
 * @param argc Number of arguments
 * @param argv Arguments
 *
 * @return 0 for success,
 *         non-zero otherwise
 */
int main(int argc, const char *argv[]) {
  int sig_figs;
  int autocorr_idx;
  std::string csv_filename;
  std::string percentiles_spec;
  std::vector<std::string> filenames;
  po::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message")(
      "sig_figs", po::value<int>(&sig_figs)->default_value(2),
      "set significant figures of output, default 2")(
      "autocorr", po::value<int>(&autocorr_idx),
      "display autocorrelations for specified chain")(
      "csv_filename", po::value<std::string>(&csv_filename),
      "write summary to csv file (also print summary to console)")(
      "percentiles",
      po::value<std::string>(&percentiles_spec)->default_value("5, 50, 95"),
      "percentiles")("input_files",
                     po::value<std::vector<std::string> >(&filenames),
                     "sampler csv files");
  po::positional_options_description p;
  p.add("input_files", -1);

  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  if (vm.count("sig_figs") && !vm["sig_figs"].defaulted()) {
    std::cout << "sig_figs " << vm["sig_figs"].as<int>() << std::endl;
  }
  if (vm.count("autocorr"))
    std::cout << autocorr_idx << std::endl;
  if (vm.count("csv_filename"))
    std::cout << "csv_filename " << csv_filename << std::endl;
  if (vm.count("percentiles") && !vm["percentiles"].defaulted()) {
    std::cout << "percentiles " << percentiles_spec << std::endl;
  }
  if (vm.count("input_files")) {
    if (filenames.size() == 1)
      std::cout << "Input file: ";
    else
      std::cout << "Input files: ";
    size_t i = 0;
    for (std::vector<std::string>::iterator it = filenames.begin();
         it != filenames.end(); it++, ++i) {
      std::cout << (*it);
      if (i < filenames.size() - 1)
        std::cout << ", ";
    }
    std::cout << std::endl;
  }

  std::vector<std::string> percentiles;
  boost::algorithm::split(percentiles, percentiles_spec, boost::is_any_of(", "),
                          boost::token_compress_on);
  Eigen::VectorXd probs = percentiles_to_probs(percentiles);

  // parse csv files into sample, metadata
  stan::io::stan_csv_metadata metadata;
  Eigen::VectorXd warmup_times(filenames.size());
  Eigen::VectorXd sampling_times(filenames.size());
  Eigen::VectorXi thin(filenames.size());
  stan::mcmc::chains<> chains = parse_csv_files(
      filenames, metadata, warmup_times, sampling_times, thin, &std::cout);

  size_t max_name_length = 0;
  size_t num_sampler_params = -1;  // don't count name 'lp__'
  for (int i = 0; i < chains.num_params(); ++i) {
    if (chains.param_name(i).length() > max_name_length)
      max_name_length = chains.param_name(i).length();
    if (stan::io::ends_with("__", chains.param_name(i)))
      num_sampler_params++;
  }
  size_t num_model_params = chains.num_params() - num_sampler_params;

  std::vector<std::string> sampler_params_hdr
      = get_sampler_params_header(percentiles);
  std::vector<std::string> model_params_hdr
      = get_model_params_header(percentiles);

  // Compute statistics for sampler and model params
  int sampler_params_start_col = 1;  // col 0 is `lp__`
  Eigen::MatrixXd sampler_params(num_sampler_params, sampler_params_hdr.size());
  sampler_params_stats(chains, probs, sampler_params_start_col, sampler_params);

  int model_params_start_col = num_sampler_params + sampler_params_start_col;
  Eigen::MatrixXd model_params(num_model_params, model_params_hdr.size());
  model_params_stats(chains, warmup_times, sampling_times, probs,
                     model_params_start_col, model_params);

  // Print to console
  timing_summary(chains, metadata, warmup_times, sampling_times, thin, "",
                 &std::cout);

  sampler_params_summary(chains, sampler_params, sampler_params_hdr,
                         sampler_params_start_col, max_name_length, sig_figs,
                         &std::cout);
  std::cout << std::endl;

  model_params_summary(chains, model_params, model_params_hdr,
                       model_params_start_col, max_name_length, sig_figs,
                       &std::cout);
  std::cout << std::endl;

  sampler_summary(metadata, "", &std::cout);
  std::cout << std::endl;

  if (vm.count("autocorr")) {
    autocorrelation(chains, metadata, autocorr_idx, max_name_length);
    std::cout << std::endl;
  }

  if (vm.count("csv_filename")) {
    std::ofstream csv_file(csv_filename.c_str(), std::ios_base::app);
    model_params_summary(chains, model_params, model_params_hdr,
                         model_params_start_col, max_name_length, sig_figs,
                         &csv_file);
    timing_summary(chains, metadata, warmup_times, sampling_times, thin, "# ",
                   &csv_file);
    sampler_summary(metadata, "# ", &csv_file);
    csv_file.close();
  }

  return 0;
}
