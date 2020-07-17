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
#include <boost/program_options.hpp>

/**
 * Compute summary statistics over HMC sampler output
 * read in from stan_csv files.
 * Command line options handled by boost::program_options.
 *
 * @param argc Number of arguments
 * @param argv Arguments
 *
 * @return 0 for success,
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
                              Must be an integer from (1, 10), inclusive.
)";
  if (argc < 2) {  // pre-empt boost::program_options
    std::cout << usage << std::endl;
    return -1;
  }

  // Command-line arguments
  int sig_figs;
  int autocorr_idx;
  std::string csv_filename;
  std::string percentiles_spec;
  std::vector<std::string> filenames;
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()("help,h", "Produce help message")(
      "sig_figs,s",
      boost::program_options::value<int>(&sig_figs)->default_value(2),
      "Significant figures, default 2.")(
      "autocorr,a", boost::program_options::value<int>(&autocorr_idx),
      "Display the chain autocorrelation.")(
      "csv_filename,c",
      boost::program_options::value<std::string>(&csv_filename),
      "Write statistics to a csv.")(
      "percentiles,p",
      boost::program_options::value<std::string>(&percentiles_spec)
          ->default_value("5,50,95"),
      "Percentiles to report.")(
      "input_files,i",
      boost::program_options::value<std::vector<std::string> >(&filenames),
      "Sampler csv files. ");
  boost::program_options::positional_options_description p;
  p.add("input_files", -1);

  // Parse, validate command-line
  boost::program_options::variables_map vm;
  try {
    boost::program_options::store(
        boost::program_options::command_line_parser(argc, argv)
            .options(desc)
            .positional(p)
            .run(),
        vm);
    boost::program_options::notify(vm);
  } catch (const boost::program_options::error &e) {
    std::cout << "Invalid argument: " << e.what() << std::endl;
    std::cout << std::endl << usage << std::endl;
    return -1;
  }
  if (vm.count("help")) {
    std::cout << std::endl << usage << std::endl;
    return 0;
  }
  if (vm.count("input_files")) {
    for (size_t i = 0; i < filenames.size(); ++i) {
      if (FILE *file = fopen(filenames[i].c_str(), "r")) {
        fclose(file);
      } else {
        std::cout << "Invalid input file: " << filenames[i] << ", exiting."
                  << std::endl;
        std::cout << std::endl << usage << std::endl;
        return -1;
      }
    }
    if (filenames.size() == 1)
      std::cout << "Input file: ";
    else
      std::cout << "Input files: ";
    for (size_t i = 0; i < filenames.size(); ++i) {
      std::cout << filenames[i];
      if (i < filenames.size() - 1)
        std::cout << ", ";
    }
    std::cout << std::endl;
  } else {
    std::cout
        << "No Stan csv file(s) specified, expecting one or more filenames."
        << std::endl;
    std::cout << std::endl << usage << std::endl;
    return -1;
  }
  if (vm.count("csv_filename")) {
    if (FILE *file = fopen(csv_filename.c_str(), "w")) {
      fclose(file);
    } else {
      std::cout << "Invalid output csv file: " << csv_filename << ", exiting."
                << std::endl;
      std::cout << std::endl << usage << std::endl;
      return -1;
    }
    std::cout << "Ouput csv_file: " << csv_filename << std::endl;
  }
  if (vm.count("sig_figs") && !vm["sig_figs"].defaulted()) {
    if (sig_figs < 1 || sig_figs > 10) {
      std::cout << "Bad value for option --sig_figs: "
                << vm["sig_figs"].as<int>() << ", exiting." << std::endl;
      std::cout << std::endl << usage << std::endl;
      return -1;
    }
    std::cout << "Significant digits: " << vm["sig_figs"].as<int>()
              << std::endl;
  }
  if (vm.count("autocorr")) {
    if (autocorr_idx < 1 || autocorr_idx > filenames.size()) {
      std::cout << "Bad value for option --autocorr: " << autocorr_idx
                << ", exiting." << std::endl;
      std::cout << std::endl << usage << std::endl;
      return -1;
    }
    std::cout << "Autocorrelation for chain: " << autocorr_idx << std::endl;
  }
  if (vm.count("percentiles") && !vm["percentiles"].defaulted()) {
    std::cout << "Percentiles: " << percentiles_spec << std::endl;
  }
  std::vector<std::string> percentiles;
  boost::algorithm::trim(
      percentiles_spec);  // split treats leading space as token
  boost::algorithm::split(percentiles, percentiles_spec, boost::is_any_of(", "),
                          boost::token_compress_on);
  Eigen::VectorXd probs;
  try {
    probs = percentiles_to_probs(percentiles);
  } catch (const boost::program_options::error &e) {
    std::cout << "Bad value for option --percentiles: " << e.what()
              << std::endl;
    std::cout << std::endl << usage << std::endl;
    return -1;
  }

  // Parse csv files into sample, metadata
  stan::io::stan_csv_metadata metadata;
  Eigen::VectorXd warmup_times(filenames.size());
  Eigen::VectorXd sampling_times(filenames.size());
  Eigen::VectorXi thin(filenames.size());
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
  model_widths
      = calculate_column_widths(model_params, header, sig_figs, model_formats);

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
  write_params(chains, lp_param, column_widths, model_formats, max_name_length,
               sig_figs, 0, false, &std::cout);
  write_params(chains, sampler_params, column_widths, sampler_formats,
               max_name_length, sig_figs, 1, false, &std::cout);
  std::cout << std::endl;
  write_params(chains, model_params, column_widths, model_formats,
               max_name_length, sig_figs, model_params_offset, false,
               &std::cout);
  std::cout << std::endl;
  write_sampler_info(metadata, "", &std::cout);

  if (vm.count("autocorr")) {
    autocorrelation(chains, metadata, autocorr_idx, max_name_length);
    std::cout << std::endl;
  }

  // Write to csv file (optional)
  if (vm.count("csv_filename")) {
    std::ofstream csv_file(csv_filename.c_str(), std::ios_base::app);
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

  return 0;
}
