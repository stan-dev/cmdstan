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

  // Parse csv files
  Eigen::VectorXd warmup_times(filenames.size());
  Eigen::VectorXd sampling_times(filenames.size());
  Eigen::VectorXi thin(filenames.size());

  // Parse first csv file in order to instantiate stan::mcmc::chains object
  std::ifstream ifstream;
  ifstream.open(filenames[0].c_str());

  stan::io::stan_csv stan_csv
      = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
  warmup_times(0) = stan_csv.timing.warmup;
  sampling_times(0) = stan_csv.timing.sampling;

  stan::mcmc::chains<> chains(stan_csv);
  ifstream.close();

  thin(0) = stan_csv.metadata.thin;

  // Parse remaining files; add to chains
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

  // Compute num sampler params, longest variable name
  std::string model_name = stan_csv.metadata.model;
  size_t max_name_length = 0;
  size_t num_diags = -1;
  for (int i = 0; i < chains.num_params(); ++i) {
    if (chains.param_name(i).length() > max_name_length)
      max_name_length = chains.param_name(i).length();
    if (stan::io::ends_with("__", chains.param_name(i)))
      num_diags++;
  }
  std::cout << "num_diags: " << num_diags << std::endl;

  // Specify percentiles
  Eigen::VectorXd probs = parse_probs(percentiles_spec);
  std::vector<std::string> percentiles;
  boost::algorithm::split(percentiles, percentiles_spec, boost::is_any_of(", "),
                          boost::token_compress_on);

  size_t num_diag_cols = probs.size() + 2;  // Mean, StdDev
  size_t num_param_cols = probs.size() + 6; // + MCSE, N_eff, N_eff/s, R_hat

  // Prepare headers
  std::vector<std::string> diag_headers(num_diag_cols);
  diag_headers.at(0) = "Mean";
  diag_headers.at(1) = "StdDev";
  size_t dh_idx = 2;
  for (std::vector<std::string>::iterator it = percentiles.begin();
       it != percentiles.end(); it++, dh_idx++) {
    diag_headers.at(dh_idx) = (*it) + '%';
  }

  std::vector<std::string> param_headers(num_param_cols);
  param_headers.at(0) = "Mean";
  param_headers.at(1) = "MCSE";
  param_headers.at(2) = "StdDev";
  size_t ph_idx = 3;
  for (std::vector<std::string>::iterator it = percentiles.begin();
       it != percentiles.end(); it++, ph_idx++) {
    param_headers.at(ph_idx) = (*it) + '%';
  }
  param_headers.at(probs.size() + 3) = "N_Eff";
  param_headers.at(probs.size() + 4) = "N_Eff/s";
  param_headers.at(probs.size() + 5) = "R_hat";

  int diags_start_col = 1;
  int params_start_col = num_diags + diags_start_col;

  Eigen::MatrixXd diag_values(num_diags, num_diag_cols);
  diag_values.setZero();
  // NUTS HMC sampler accecpt_stat__ , ..., energy__
  for (int i = 0; i < num_diags; ++i) {
    int i_offset = i + diags_start_col;
    diag_values(i, 0) = chains.mean(i_offset);
    diag_values(i, 1) = chains.sd(i_offset);
    Eigen::VectorXd quantiles = chains.quantiles(i_offset, probs);
    for (int j = 0; j < probs.size(); j++)
      diag_values(i, 2 + j) = quantiles(j);
  }

  Eigen::MatrixXd param_values(chains.num_params() - num_diags, num_param_cols);
  param_values.setZero();
  // Joint log prob lp__
  param_values(0, 0) = chains.mean(0);
  double lp_sd = chains.sd(0);
  double lp_n_eff = chains.effective_sample_size(0);
  param_values(0, 1) = lp_sd / sqrt(lp_n_eff);
  param_values(0, 2) = lp_sd;
  Eigen::VectorXd quantiles = chains.quantiles(0, probs);
  for (int j = 0; j < probs.size(); j++)
    param_values(0, 3 + j) = quantiles(j);
  param_values(0, probs.size() + 3) = lp_n_eff;
  param_values(0, probs.size() + 4) = lp_n_eff / total_sampling_time;
  param_values(0, probs.size() + 5) = chains.split_potential_scale_reduction(0);
  // Model parameters
  for (int i = params_start_col; i < chains.num_params(); ++i) {
    //    std::cout << "param: " << i <<  " " << chains.param_name(i) <<
    //    std::endl;
    int i_offset = i - num_diags;
    double sd = chains.sd(i_offset);
    double n_eff = chains.effective_sample_size(i);
    param_values(i_offset, 0) = chains.mean(i);
    param_values(i_offset, 1) = sd / sqrt(n_eff);
    param_values(i_offset, 2) = sd;
    Eigen::VectorXd quantiles = chains.quantiles(i, probs);
    for (int j = 0; j < probs.size(); j++)
      param_values(i_offset, 3 + j) = quantiles(j);
    param_values(i_offset, probs.size() + 3) = n_eff;
    param_values(i_offset, probs.size() + 4) = n_eff / total_sampling_time;
    param_values(i_offset, probs.size() + 5)
        = chains.split_potential_scale_reduction(i);
  }

  // Model, chains
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


  // Compute column widths
  Eigen::VectorXi column_sig_figs(num_param_cols);
  Eigen::VectorXi column_widths(num_param_cols);
  Eigen::Matrix<std::ios_base::fmtflags, Eigen::Dynamic, 1> formats(
      num_param_cols);
  column_widths
      = calculate_column_widths(param_values, param_headers, sig_figs, formats);

  // Diags summary
  std::cout << std::setw(max_name_length + 1) << "";
  for (int i = 0; i < num_diag_cols; ++i) {
    std::cout << std::setw(column_widths(i)) << diag_headers[i];
  }
  std::cout << std::endl;

  // Diags values
  for (int i = 0; i < num_diags; ++i) {
    int i_offset = i + diags_start_col;
    std::cout << std::setw(max_name_length + 1) << std::left
              << chains.param_name(i_offset);
    std::cout << std::right;
    for (int j = 0; j < num_diag_cols; j++) {
      std::cout.setf(formats(j), std::ios::floatfield);
      std::cout << std::setprecision(compute_precision(
          diag_values(i, j), sig_figs, formats(j) == std::ios_base::scientific))
                << std::setw(column_widths(j)) << diag_values(i, j);
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;

  // Params summary
  std::cout << std::setw(max_name_length + 1) << "";
  for (int i = 0; i < num_param_cols; ++i) {
    std::cout << std::setw(column_widths(i)) << param_headers[i];
  }
  std::cout << std::endl;

  // Joint log prob lp__
  std::cout << std::setw(max_name_length + 1) << std::left
            << chains.param_name(0);
  std::cout << std::right;
  for (int j = 0; j < num_param_cols; j++) {
    std::cout.setf(formats(j), std::ios::floatfield);
    std::cout << std::setprecision(compute_precision(
        param_values(0, j), sig_figs, formats(j) == std::ios_base::scientific))
              << std::setw(column_widths(j)) << param_values(0, j);
  }
  std::cout << std::endl;
  
  // Model params - already reported lp__, start from 1
  for (int i = 1; i < chains.num_params() - num_diags; ++i) {
    int i_offset = i + num_diags;
    //    std::cout << "i: " << i << " i_offset: " << i_offset << std::endl;
    if (!is_matrix(chains.param_name(i_offset))) {
      // scalar param is at position i_offset in chains
      std::cout << std::setw(max_name_length + 1) << std::left
                << chains.param_name(i_offset);
      std::cout << std::right;
      for (int j = 0; j < num_param_cols; j++) {
        std::cout.setf(formats(j), std::ios::floatfield);
        std::cout << std::setprecision(
            compute_precision(param_values(i, j), sig_figs,
                              formats(j) == std::ios_base::scientific))
                  << std::setw(column_widths(j)) << param_values(i, j);
      }
      std::cout << std::endl;
    } else {
      // container object columns in csv are last-index-major order
      // output as first-index-major order
      std::vector<int> dims = dimensions(chains, i_offset);
      std::vector<int> index(dims.size(), 1);
      int max = 1;
      for (size_t j = 0; j < dims.size(); j++)
        max *= dims[j];
      //      std::cout << "max: " << max << std::endl;
      for (int k = 0; k < max; k++) {
        //        std::cout << "k: " << k << std::endl;
        int row_maj_index = i_offset + matrix_index(index, dims);
        //        std::cout << "row_maj_index: " << row_maj_index << std::endl;
        std::cout << std::setw(max_name_length + 1) << std::left
                  << chains.param_name(row_maj_index);
        std::cout << std::right;
        for (int j = 0; j < num_param_cols; j++) {
          std::cout.setf(formats(j), std::ios::floatfield);
          std::cout << std::setprecision(compute_precision(
              param_values(row_maj_index - num_diags, j), sig_figs,
              formats(j) == std::ios_base::scientific))
                    << std::setw(column_widths(j))
                    << param_values(row_maj_index - num_diags, j);
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

  // // Print autocorrelation, if desired
  // if (vm.count("autocorr")) {
  //   const int c = autocorr_idx;
  //   if (c < 0 || c >= chains.num_chains()) {
  //     std::cout << "Bad chain index " << c
  //               << ", aborting autocorrelation display." << std::endl;
  //   } else {
  //     Eigen::MatrixXd autocorr(chains.num_params(), chains.num_samples(c));

  //     for (int i = 0; i < chains.num_params(); ++i) {
  //       autocorr.row(i) = chains.autocorrelation(c, i);
  //     }

  //     // Format and print header
  //     std::cout << "Displaying the autocorrelations for chain " << c << ":"
  //               << std::endl;
  //     std::cout << std::endl;

  //     const int n_autocorr = autocorr.row(0).size();

  //     int lag_width = 1;
  //     int number = n_autocorr;
  //     while (number != 0) {
  //       number /= 10;
  //       lag_width++;
  //     }

  //     std::cout << std::setw(lag_width > 4 ? lag_width : 4) << "Lag";
  //     for (int i = 0; i < chains.num_params(); ++i) {
  //       std::cout << std::setw(max_name_length + 1) << std::right
  //                 << chains.param_name(i);
  //     }
  //     std::cout << std::endl;

  //     // Print body
  //     for (int n = 0; n < n_autocorr; ++n) {
  //       std::cout << std::setw(lag_width) << std::right << n;
  //       for (int i = 0; i < chains.num_params(); ++i) {
  //         std::cout << std::setw(max_name_length + 1) << std::right
  //                   << autocorr(i, n);
  //       }
  //       std::cout << std::endl;
  //     }
  //   }
  // }

  // if (vm.count("csv_filename")) {
  //   std::ofstream csv_file(csv_filename.c_str(), std::ios_base::app);

  //   // Header output
  //   csv_file << "name";
  //   for (int i = 0; i < num_cols; ++i)
  //     csv_file << "," << headers[i];
  //   csv_file << std::endl;

  //   // Value output
  //   for (int i = 0; i < chains.num_params(); ++i) {
  //     if (!is_matrix(chains.param_name(i))) {
  //       csv_file << "\"" << chains.param_name(i) << "\"";
  //       for (int j = 0; j < num_cols; j++) {
  //         csv_file << "," << values(i, j);
  //       }
  //       csv_file << std::endl;
  //     } else {
  //       std::vector<int> dims = dimensions(chains, i);
  //       std::vector<int> index(dims.size(), 1);
  //       int max = 1;
  //       for (size_t j = 0; j < dims.size(); j++)
  //         max *= dims[j];

  //       for (int k = 0; k < max; k++) {
  //         int param_index = i + matrix_index(index, dims);
  //         csv_file << "\"" << chains.param_name(param_index) << "\"";
  //         for (int j = 0; j < num_cols; j++)
  //           csv_file << "," << values(param_index, j);
  //         csv_file << std::endl;
  //         if (k < max - 1)
  //           next_index(index, dims);
  //       }
  //       i += max - 1;
  //     }
  //   }

  //   // comments
  //   // Initial output
  //   csv_file << "# Inference for Stan model: " << model_name << std::endl
  //            << "# " << chains.num_chains() << " chains: each with iter=("
  //            << chains.num_kept_samples(0);
  //   for (int chain = 1; chain < chains.num_chains(); chain++)
  //     csv_file << "," << chains.num_kept_samples(chain);
  //   csv_file << ")";

  //   // Timing output
  //   csv_file << "; warmup=(" << chains.warmup(0);
  //   for (int chain = 1; chain < chains.num_chains(); chain++)
  //     csv_file << "," << chains.warmup(chain);
  //   csv_file << ")";

  //   csv_file << "; thin=(" << thin(0);

  //   for (int chain = 1; chain < chains.num_chains(); chain++)
  //     csv_file << "," << thin(chain);
  //   csv_file << ")";

  //   csv_file << "; " << chains.num_samples() << " iterations saved."
  //            << std::endl
  //            << "#" << std::endl;

  //   std::string warmup_unit = "seconds";

  //   if (total_warmup_time / 3600 > 1) {
  //     total_warmup_time /= 3600;
  //     warmup_unit = "hours";
  //   } else if (total_warmup_time / 60 > 1) {
  //     total_warmup_time /= 60;
  //     warmup_unit = "minutes";
  //   }

  //   csv_file << "# Warmup took (" << std::fixed
  //            << std::setprecision(
  //                   compute_precision(warmup_times(0), sig_figs, false))
  //            << warmup_times(0);
  //   for (int chain = 1; chain < chains.num_chains(); chain++)
  //     csv_file << ", " << std::fixed
  //              << std::setprecision(
  //                     compute_precision(warmup_times(chain), sig_figs, false))
  //              << warmup_times(chain);
  //   csv_file << ") seconds, ";
  //   csv_file << std::fixed
  //            << std::setprecision(
  //                   compute_precision(total_warmup_time, sig_figs, false))
  //            << total_warmup_time << " " << warmup_unit << " total"
  //            << std::endl;

  //   std::string sampling_unit = "seconds";

  //   if (total_sampling_time / 3600 > 1) {
  //     total_sampling_time /= 3600;
  //     sampling_unit = "hours";
  //   } else if (total_sampling_time / 60 > 1) {
  //     total_sampling_time /= 60;
  //     sampling_unit = "minutes";
  //   }

  //   csv_file << "# Sampling took (" << std::fixed
  //            << std::setprecision(
  //                   compute_precision(sampling_times(0), sig_figs, false))
  //            << sampling_times(0);
  //   for (int chain = 1; chain < chains.num_chains(); chain++)
  //     csv_file << ", " << std::fixed
  //              << std::setprecision(
  //                     compute_precision(sampling_times(chain), sig_figs, false))
  //              << sampling_times(chain);
  //   csv_file << ") seconds, ";
  //   csv_file << std::fixed
  //            << std::setprecision(
  //                   compute_precision(total_sampling_time, sig_figs, false))
  //            << total_sampling_time << " " << sampling_unit << " total"
  //            << std::endl;
  //   csv_file << "#" << std::endl;

  //   /// Footer output
  //   csv_file << "# Samples were drawn using " << stan_csv.metadata.algorithm
  //            << " with " << stan_csv.metadata.engine << "." << std::endl
  //            << "# For each parameter, N_Eff is a crude measure of effective "
  //               "sample size,"
  //            << std::endl
  //            << "# and R_hat is the potential scale reduction factor on split "
  //               "chains (at "
  //            << std::endl
  //            << "# convergence, R_hat=1)." << std::endl;

  //   // Print autocorrelation, if desired
  //   if (vm.count("autocorr")) {
  //     const int c = autocorr_idx;

  //     Eigen::MatrixXd autocorr(chains.num_params(), chains.num_samples(c));

  //     for (int i = 0; i < chains.num_params(); ++i) {
  //       autocorr.row(i) = chains.autocorrelation(c, i);
  //     }

  //     // Format and print header
  //     csv_file << "# Displaying the autocorrelations for chain " << c << ":"
  //              << std::endl;
  //     const int n_autocorr = autocorr.row(0).size();

  //     int lag_width = 1;
  //     int number = n_autocorr;
  //     while (number != 0) {
  //       number /= 10;
  //       lag_width++;
  //     }

  //     csv_file << "# " << std::setw(lag_width > 4 ? lag_width : 4) << "Lag";
  //     for (int i = 0; i < chains.num_params(); ++i) {
  //       csv_file << std::setw(max_name_length + 1) << std::right
  //                << chains.param_name(i);
  //     }
  //     csv_file << std::endl;

  //     // Print body
  //     for (int n = 0; n < n_autocorr; ++n) {
  //       csv_file << "# " << std::setw(lag_width) << std::right << n;
  //       for (int i = 0; i < chains.num_params(); ++i) {
  //         csv_file << std::setw(max_name_length + 1) << std::right
  //                  << autocorr(i, n);
  //       }
  //       csv_file << std::endl;
  //     }
  //   }
  //   csv_file.close();
  // }

  return 0;
}
