#include <cmdstan/stansummary_helper.hpp>
#include <stan/mcmc/chains.hpp>
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

  if (vm.count("sig_figs")) {
    std::cout << "sig_figs " << vm["sig_figs"].as<int>();
    if (vm["sig_figs"].defaulted())
      std::cout << " (default)";
    std::cout << std::endl;
  }
  if (vm.count("autocorr"))
    std::cout << autocorr_idx << std::endl;
  if (vm.count("csv_filename"))
    std::cout << "csv_filename " << csv_filename << std::endl;
  if (vm.count("percentiles")) {
    std::cout << "percentiles " << percentiles_spec;
    if (vm["sig_figs"].defaulted())
      std::cout << " (default)";
    std::cout << std::endl;
  }
  if (vm.count("input_files")) {
    if (filenames.size() == 1)
      std::cout << "input_file ";
    else
      std::cout << "input_files ";
    size_t i = 0;
    for (std::vector<std::string>::iterator it = filenames.begin();
         it != filenames.end(); it++, ++i) {
      std::cout << (*it);
      if (i < filenames.size() - 1)
        std::cout << ", ";
    }
    std::cout << std::endl;
  }

  parse_probs(percentiles_spec);

  // Parse specified files
  Eigen::VectorXd warmup_times(filenames.size());
  Eigen::VectorXd sampling_times(filenames.size());

  Eigen::VectorXi thin(filenames.size());

  std::ifstream ifstream;
  ifstream.open(filenames[0].c_str());

  stan::io::stan_csv stan_csv
      = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
  warmup_times(0) = stan_csv.timing.warmup;
  sampling_times(0) = stan_csv.timing.sampling;

  stan::mcmc::chains<> chains(stan_csv);
  ifstream.close();

  thin(0) = stan_csv.metadata.thin;

  for (std::vector<std::string>::size_type chain = 1; chain < filenames.size();
       chain++) {
    ifstream.open(filenames[chain].c_str());
    stan_csv = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
    chains.add(stan_csv);
    ifstream.close();
    thin(chain) = stan_csv.metadata.thin;

    warmup_times(chain) = stan_csv.timing.warmup;
    sampling_times(chain) = stan_csv.timing.sampling;
  }

  double total_warmup_time = warmup_times.sum();
  double total_sampling_time = sampling_times.sum();

  // Compute largest variable name length
  const int skip = 0;
  std::string model_name = stan_csv.metadata.model;
  size_t max_name_length = 0;
  for (int i = skip; i < chains.num_params(); i++)
    if (chains.param_name(i).length() > max_name_length)
      max_name_length = chains.param_name(i).length();
  for (int i = 0; i < 2; i++)
    if (chains.param_name(i).length() > max_name_length)
      max_name_length = chains.param_name(i).length();

  // Specify percentiles
  Eigen::VectorXd probs = parse_probs(percentiles_spec);

  // Prepare header
  int num_cols = probs.size() + 6;
  std::vector<std::string> headers(num_cols);
  headers.at(0) = "Mean";
  headers.at(1) = "MCSE";
  headers.at(2) = "StdDev";
  std::vector<std::string> percentiles;
  boost::algorithm::split(percentiles, percentiles_spec, boost::is_any_of(", "),
                          boost::token_compress_on);
  size_t idx = 3;
  for (std::vector<std::string>::iterator it = percentiles.begin();
       it != percentiles.end(); it++, idx++) {
    headers.at(idx) = (*it) + '%';
  }
  headers.at(probs.size() + 3) = "N_Eff";
  headers.at(probs.size() + 4) = "N_Eff/s";
  headers.at(probs.size() + 5) = "R_hat";

  // Prepare values
  Eigen::MatrixXd values(chains.num_params(), num_cols);
  values.setZero();

  for (int i = 0; i < chains.num_params(); i++) {
    double sd = chains.sd(i);
    double n_eff = chains.effective_sample_size(i);
    values(i, 0) = chains.mean(i);
    values(i, 1) = sd / sqrt(n_eff);
    values(i, 2) = sd;
    Eigen::VectorXd quantiles = chains.quantiles(i, probs);
    for (int j = 0; j < probs.size(); j++)
      values(i, 3 + j) = quantiles(j);
    values(i, probs.size() + 3) = n_eff;
    values(i, probs.size() + 4) = n_eff / total_sampling_time;
    values(i, probs.size() + 5) = chains.split_potential_scale_reduction(i);
  }

  // Compute column widths
  Eigen::VectorXi column_sig_figs(num_cols);
  Eigen::VectorXi column_widths(num_cols);
  Eigen::Matrix<std::ios_base::fmtflags, Eigen::Dynamic, 1> formats(num_cols);
  column_widths = calculate_column_widths(values, headers, sig_figs, formats);

  // Initial output
  std::cout << "Inference for Stan model: " << model_name << std::endl
            << chains.num_chains() << " chains: each with iter=("
            << chains.num_kept_samples(0);
  for (int chain = 1; chain < chains.num_chains(); chain++)
    std::cout << "," << chains.num_kept_samples(chain);
  std::cout << ")";

  // Timing output
  std::cout << "; warmup=(" << chains.warmup(0);
  for (int chain = 1; chain < chains.num_chains(); chain++)
    std::cout << "," << chains.warmup(chain);
  std::cout << ")";

  std::cout << "; thin=(" << thin(0);

  for (int chain = 1; chain < chains.num_chains(); chain++)
    std::cout << "," << thin(chain);
  std::cout << ")";

  std::cout << "; " << chains.num_samples() << " iterations saved." << std::endl
            << std::endl;

  std::string warmup_unit = "seconds";

  if (total_warmup_time / 3600 > 1) {
    total_warmup_time /= 3600;
    warmup_unit = "hours";
  } else if (total_warmup_time / 60 > 1) {
    total_warmup_time /= 60;
    warmup_unit = "minutes";
  }

  std::cout << "Warmup took (" << std::fixed
            << std::setprecision(
                   compute_precision(warmup_times(0), sig_figs, false))
            << warmup_times(0);
  for (int chain = 1; chain < chains.num_chains(); chain++)
    std::cout << ", " << std::fixed
              << std::setprecision(
                     compute_precision(warmup_times(chain), sig_figs, false))
              << warmup_times(chain);
  std::cout << ") seconds, ";
  std::cout << std::fixed
            << std::setprecision(
                   compute_precision(total_warmup_time, sig_figs, false))
            << total_warmup_time << " " << warmup_unit << " total" << std::endl;

  std::string sampling_unit = "seconds";

  if (total_sampling_time / 3600 > 1) {
    total_sampling_time /= 3600;
    sampling_unit = "hours";
  } else if (total_sampling_time / 60 > 1) {
    total_sampling_time /= 60;
    sampling_unit = "minutes";
  }

  std::cout << "Sampling took (" << std::fixed
            << std::setprecision(
                   compute_precision(sampling_times(0), sig_figs, false))
            << sampling_times(0);
  for (int chain = 1; chain < chains.num_chains(); chain++)
    std::cout << ", " << std::fixed
              << std::setprecision(
                     compute_precision(sampling_times(chain), sig_figs, false))
              << sampling_times(chain);
  std::cout << ") seconds, ";
  std::cout << std::fixed
            << std::setprecision(
                   compute_precision(total_sampling_time, sig_figs, false))
            << total_sampling_time << " " << sampling_unit << " total"
            << std::endl;
  std::cout << std::endl;

  // Header output
  std::cout << std::setw(max_name_length + 1) << "";
  for (int i = 0; i < num_cols; i++) {
    std::cout << std::setw(column_widths(i)) << headers[i];
  }
  std::cout << std::endl;

  // Value output
  for (int i = skip; i < chains.num_params(); i++) {
    if (!is_matrix(chains.param_name(i))) {
      std::cout << std::setw(max_name_length + 1) << std::left
                << chains.param_name(i);
      std::cout << std::right;
      for (int j = 0; j < num_cols; j++) {
        std::cout.setf(formats(j), std::ios::floatfield);
        std::cout << std::setprecision(compute_precision(
                         values(i, j), sig_figs,
                         formats(j) == std::ios_base::scientific))
                  << std::setw(column_widths(j)) << values(i, j);
      }
      std::cout << std::endl;
    } else {
      std::vector<int> dims = dimensions(chains, i);
      std::vector<int> index(dims.size(), 1);
      int max = 1;
      for (size_t j = 0; j < dims.size(); j++)
        max *= dims[j];

      for (int k = 0; k < max; k++) {
        int param_index = i + matrix_index(index, dims);
        std::cout << std::setw(max_name_length + 1) << std::left
                  << chains.param_name(param_index);
        std::cout << std::right;
        for (int j = 0; j < num_cols; j++) {
          std::cout.setf(formats(j), std::ios::floatfield);
          std::cout << std::setprecision(compute_precision(
                           values(param_index, j), sig_figs,
                           formats(j) == std::ios_base::scientific))
                    << std::setw(column_widths(j)) << values(param_index, j);
        }
        std::cout << std::endl;
        if (k < max - 1)
          next_index(index, dims);
      }
      i += max - 1;
    }
  }

  /// Footer output
  std::cout << std::endl;
  std::cout << "Samples were drawn using " << stan_csv.metadata.algorithm
            << " with " << stan_csv.metadata.engine << "." << std::endl
            << "For each parameter, N_Eff is a crude measure of effective "
               "sample size,"
            << std::endl
            << "and R_hat is the potential scale reduction factor on split "
               "chains (at "
            << std::endl
            << "convergence, R_hat=1)." << std::endl
            << std::endl;

  // Print autocorrelation, if desired
  if (vm.count("autocorr")) {
    const int c = autocorr_idx;
    if (c < 0 || c >= chains.num_chains()) {
      std::cout << "Bad chain index " << c
                << ", aborting autocorrelation display." << std::endl;
    } else {
      Eigen::MatrixXd autocorr(chains.num_params(), chains.num_samples(c));

      for (int i = 0; i < chains.num_params(); i++) {
        autocorr.row(i) = chains.autocorrelation(c, i);
      }

      // Format and print header
      std::cout << "Displaying the autocorrelations for chain " << c << ":"
                << std::endl;
      std::cout << std::endl;

      const int n_autocorr = autocorr.row(0).size();

      int lag_width = 1;
      int number = n_autocorr;
      while (number != 0) {
        number /= 10;
        lag_width++;
      }

      std::cout << std::setw(lag_width > 4 ? lag_width : 4) << "Lag";
      for (int i = 0; i < chains.num_params(); ++i) {
        std::cout << std::setw(max_name_length + 1) << std::right
                  << chains.param_name(i);
      }
      std::cout << std::endl;

      // Print body
      for (int n = 0; n < n_autocorr; ++n) {
        std::cout << std::setw(lag_width) << std::right << n;
        for (int i = 0; i < chains.num_params(); ++i) {
          std::cout << std::setw(max_name_length + 1) << std::right
                    << autocorr(i, n);
        }
        std::cout << std::endl;
      }
    }
  }

  if (vm.count("csv_filename")) {
    std::ofstream csv_file(csv_filename.c_str(), std::ios_base::app);

    // Header output
    csv_file << "name";
    for (int i = 0; i < num_cols; i++)
      csv_file << "," << headers[i];
    csv_file << std::endl;

    // Value output
    for (int i = skip; i < chains.num_params(); i++) {
      if (!is_matrix(chains.param_name(i))) {
        csv_file << "\"" << chains.param_name(i) << "\"";
        for (int j = 0; j < num_cols; j++) {
          csv_file << "," << values(i, j);
        }
        csv_file << std::endl;
      } else {
        std::vector<int> dims = dimensions(chains, i);
        std::vector<int> index(dims.size(), 1);
        int max = 1;
        for (size_t j = 0; j < dims.size(); j++)
          max *= dims[j];

        for (int k = 0; k < max; k++) {
          int param_index = i + matrix_index(index, dims);
          csv_file << "\"" << chains.param_name(param_index) << "\"";
          for (int j = 0; j < num_cols; j++)
            csv_file << "," << values(param_index, j);
          csv_file << std::endl;
          if (k < max - 1)
            next_index(index, dims);
        }
        i += max - 1;
      }
    }

    // comments
    // Initial output
    csv_file << "# Inference for Stan model: " << model_name << std::endl
             << "# " << chains.num_chains() << " chains: each with iter=("
             << chains.num_kept_samples(0);
    for (int chain = 1; chain < chains.num_chains(); chain++)
      csv_file << "," << chains.num_kept_samples(chain);
    csv_file << ")";

    // Timing output
    csv_file << "; warmup=(" << chains.warmup(0);
    for (int chain = 1; chain < chains.num_chains(); chain++)
      csv_file << "," << chains.warmup(chain);
    csv_file << ")";

    csv_file << "; thin=(" << thin(0);

    for (int chain = 1; chain < chains.num_chains(); chain++)
      csv_file << "," << thin(chain);
    csv_file << ")";

    csv_file << "; " << chains.num_samples() << " iterations saved."
             << std::endl
             << "#" << std::endl;

    std::string warmup_unit = "seconds";

    if (total_warmup_time / 3600 > 1) {
      total_warmup_time /= 3600;
      warmup_unit = "hours";
    } else if (total_warmup_time / 60 > 1) {
      total_warmup_time /= 60;
      warmup_unit = "minutes";
    }

    csv_file << "# Warmup took (" << std::fixed
             << std::setprecision(
                    compute_precision(warmup_times(0), sig_figs, false))
             << warmup_times(0);
    for (int chain = 1; chain < chains.num_chains(); chain++)
      csv_file << ", " << std::fixed
               << std::setprecision(
                      compute_precision(warmup_times(chain), sig_figs, false))
               << warmup_times(chain);
    csv_file << ") seconds, ";
    csv_file << std::fixed
             << std::setprecision(
                    compute_precision(total_warmup_time, sig_figs, false))
             << total_warmup_time << " " << warmup_unit << " total"
             << std::endl;

    std::string sampling_unit = "seconds";

    if (total_sampling_time / 3600 > 1) {
      total_sampling_time /= 3600;
      sampling_unit = "hours";
    } else if (total_sampling_time / 60 > 1) {
      total_sampling_time /= 60;
      sampling_unit = "minutes";
    }

    csv_file << "# Sampling took (" << std::fixed
             << std::setprecision(
                    compute_precision(sampling_times(0), sig_figs, false))
             << sampling_times(0);
    for (int chain = 1; chain < chains.num_chains(); chain++)
      csv_file << ", " << std::fixed
               << std::setprecision(
                      compute_precision(sampling_times(chain), sig_figs, false))
               << sampling_times(chain);
    csv_file << ") seconds, ";
    csv_file << std::fixed
             << std::setprecision(
                    compute_precision(total_sampling_time, sig_figs, false))
             << total_sampling_time << " " << sampling_unit << " total"
             << std::endl;
    csv_file << "#" << std::endl;

    /// Footer output
    csv_file << "# Samples were drawn using " << stan_csv.metadata.algorithm
             << " with " << stan_csv.metadata.engine << "." << std::endl
             << "# For each parameter, N_Eff is a crude measure of effective "
                "sample size,"
             << std::endl
             << "# and R_hat is the potential scale reduction factor on split "
                "chains (at "
             << std::endl
             << "# convergence, R_hat=1)." << std::endl;

    // Print autocorrelation, if desired
    if (vm.count("autocorr")) {
      const int c = autocorr_idx;

      Eigen::MatrixXd autocorr(chains.num_params(), chains.num_samples(c));

      for (int i = 0; i < chains.num_params(); i++) {
        autocorr.row(i) = chains.autocorrelation(c, i);
      }

      // Format and print header
      csv_file << "# Displaying the autocorrelations for chain " << c << ":"
               << std::endl;
      const int n_autocorr = autocorr.row(0).size();

      int lag_width = 1;
      int number = n_autocorr;
      while (number != 0) {
        number /= 10;
        lag_width++;
      }

      csv_file << "# " << std::setw(lag_width > 4 ? lag_width : 4) << "Lag";
      for (int i = 0; i < chains.num_params(); ++i) {
        csv_file << std::setw(max_name_length + 1) << std::right
                 << chains.param_name(i);
      }
      csv_file << std::endl;

      // Print body
      for (int n = 0; n < n_autocorr; ++n) {
        csv_file << "# " << std::setw(lag_width) << std::right << n;
        for (int i = 0; i < chains.num_params(); ++i) {
          csv_file << std::setw(max_name_length + 1) << std::right
                   << autocorr(i, n);
        }
        csv_file << std::endl;
      }
    }
    csv_file.close();
  }

  return 0;
}
