#ifndef CMDSTAN_STANSUMMARY_HELPER_HPP
#define CMDSTAN_STANSUMMARY_HELPER_HPP

#include <stan/mcmc/chainset.hpp>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ios>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

/**
 * Determine size, and number of decimals required
 * to display a value to a given level of significance.
 *
 * @param in value to be displayed
 * @param in significant digits required
 * @param out column width
 * @param out number of decimal places
 */
void compute_width_and_precision(double value, int sig_figs, int &width,
                                 int &precision) {
  double abs_value = std::fabs(value);

  if (value == 0) {
    width = sig_figs;
    precision = sig_figs;
  } else if (std::isnan(value)) {
    width = 3;
    precision = sig_figs;
  } else if (abs_value >= 1) {
    int int_part = static_cast<int>(std::ceil(log10(abs_value) + 1e-6));
    width = int_part >= sig_figs ? int_part : sig_figs + 1;
    precision = int_part >= sig_figs ? 0 : sig_figs - int_part;
  } else {
    int frac_part = static_cast<int>(std::fabs(std::floor(log10(abs_value))));
    width = 1 + frac_part + sig_figs;
    precision = frac_part + sig_figs - 1;
  }

  // account for negative numbers
  if (std::signbit(value))
    ++width;
}

/**
 * Return size of output slot required
 * to display a value to a given level of significance.
 *
 * @param in value to be displayed
 * @param in significant digits required
 * @return display width
 */
int compute_width(double value, int sig_figs) {
  int width;
  int precision;
  compute_width_and_precision(value, sig_figs, width, precision);
  return width;
}

/**
 * Return number of decimal places required
 * to display a value to a given level of significance.
 *
 * @param in value to be displayed
 * @param in significant digits required
 * @return number of decimal places
 */
int compute_precision(double value, int sig_figs, bool scientific) {
  if (scientific) {
    return sig_figs - 1;
  } else {
    int width;
    int precision;
    compute_width_and_precision(value, sig_figs, width, precision);
    return precision;
  }
}

/**
 * Return column width required to display all values in that column
 * to a given level of significance.
 *
 * @param in vector of values to be displayed
 * @param in column label
 * @param in significant digits required
 * @param out format flags
 * @return column width
 */
int column_width(const Eigen::VectorXd &x, const std::string &name,
                 int sig_figs, std::ios_base::fmtflags &format) {
  int padding = 2;

  // Fixed Precision
  size_t fixed_threshold = 10;
  size_t max_fixed_width = 0;

  for (int i = 0; i < x.size(); ++i) {
    size_t width = compute_width(x[i], sig_figs);
    max_fixed_width = width > max_fixed_width ? width : max_fixed_width;
  }

  if (max_fixed_width + padding < fixed_threshold) {
    format = std::ios_base::fixed;
    max_fixed_width
        = name.length() > max_fixed_width ? name.length() : max_fixed_width;
    return max_fixed_width + padding;
  }

  // Scientific Notation
  size_t scientific_width = sig_figs + 1 + 4;  // Decimal place + exponent
  if (x.minCoeff() < 0)
    ++scientific_width;

  scientific_width
      = name.length() > scientific_width ? name.length() : scientific_width;

  format = std::ios_base::scientific;
  return scientific_width + padding;
}

using Eigen::Dynamic;

/**
 * Determine column widths and format flags required to display all values
 * across all columns of a data matrix, given the data, a vector of labels,
 * and the desired level of significance.
 *
 * @param in matrix of data values to be displayed
 * @param in vector of column labels
 * @param in significant digits required
 * @param out vector of format flags
 * @return vector of column widths
 */
Eigen::VectorXi calculate_column_widths(
    const Eigen::MatrixXd &values, const std::vector<std::string> &headers,
    int sig_figs, Eigen::Matrix<std::ios_base::fmtflags, Dynamic, 1> &formats) {
  int n = values.cols();
  Eigen::VectorXi column_widths(n);
  formats.resize(n);
  for (int i = 0; i < n; i++) {
    column_widths(i)
        = column_width(values.col(i), headers[i], sig_figs, formats(i));
  }
  return column_widths;
}

/**
 * Given a column label, determine whether or not the parameter
 * is a scalar variable or a container variable.
 *
 * @param in column label
 * @return boolean
 */
bool is_container(const std::string &parameter_name) {
  return (parameter_name.find("[") != std::string::npos);
}

/**
 * Return parameter name corresponding to column label.
 *
 * @param in column index
 * @return variable name
 */
std::string base_param_name(const stan::mcmc::chainset<> &chains, int index) {
  std::string name = chains.param_name(index);
  return name.substr(0, name.find("["));
}

/**
 * Return parameter name corresponding to column label.
 *
 * @param in set of samples from one or more chains
 * @param in column index
 * @return parameter name
 */
std::string matrix_index(const stan::mcmc::chainset<> &chains, int index) {
  std::string name = chains.param_name(index);
  return name.substr(name.find("["));
}

/**
 * Return vector of dimensions for container variable.
 *
 * @param in set of samples from one or more chains
 * @param in column index of first container element
 * @return vector of dimensions
 */
std::vector<int> dimensions(const stan::mcmc::chainset<> &chains,
                            int start_index) {
  std::vector<int> dims;
  int dim;

  std::string name = base_param_name(chains, start_index);
  int last_matrix_element = start_index;
  while (last_matrix_element + 1 < chains.num_params()) {
    if (base_param_name(chains, last_matrix_element + 1) == name)
      last_matrix_element++;
    else
      break;
  }

  std::stringstream ss(matrix_index(chains, last_matrix_element));
  ss.get();
  ss >> dim;

  dims.push_back(dim);
  while (ss.get() == ',') {
    ss >> dim;
    dims.push_back(dim);
  }
  return dims;
}

/**
 * Compute index for next container element,
 * for row-major order traversal.
 *
 * <p>Stan program stores/output container elements in column-major order.
 * Legacy code to manipulate indices accordingly.
 *
 * @param in out container element indices
 * @param in vector of array dimensions
 */
void next_index(std::vector<int> &index, const std::vector<int> &dims) {
  if (dims.size() != index.size())
    throw std::domain_error("next_index: size mismatch");
  if (dims.size() == 0)
    return;
  index[index.size() - 1]++;

  for (int i = index.size() - 1; i > 0; i--) {
    if (index[i] > dims[i]) {
      index[i - 1]++;
      index[i] = 1;
    }
  }

  for (size_t n = 0; n < dims.size(); n++) {
    if (index[n] <= 0 || index[n] > dims[n]) {
      std::stringstream message_stream("");
      message_stream << "next_index: index[" << n << "] out of bounds. "
                     << "dims[" << n << "] = " << dims[n] << "; "
                     << "index[" << n << "] = " << index[n];
      throw std::domain_error(message_stream.str());
    }
  }
}

/**
 * Return the flat 0-based index of a column major order matrix based on the
 * 1-based index
 *
 * @param in out container element indices
 * @param in vector of array dimensions
 * @return offset from first container element.
 */
int matrix_index(std::vector<int> &index, const std::vector<int> &dims) {
  if (dims.size() != index.size())
    throw std::domain_error("next_index: size mismatch");
  if (dims.size() == 0)
    return 0;
  for (size_t n = 0; n < dims.size(); n++) {
    if (index[n] <= 0 || index[n] > dims[n]) {
      std::stringstream message_stream("");
      message_stream << "matrix_index: index[" << n << "] out of bounds. "
                     << "dims[" << n << "] = " << dims[n] << "; "
                     << "index[" << n << "] = " << index[n];
      throw std::domain_error(message_stream.str());
    }
  }

  int offset = 0;
  int prod = 1;
  for (size_t i = 0; i < dims.size(); i++) {
    offset += (index[i] - 1) * prod;
    prod *= dims[i];
  }
  return offset;
}

/**
 * Convert percentiles - int values in range (1,99)
 * to probabilities - double values in range (0, 1).
 *
 * <p>Input values must be in strictly increasing order.
 *
 * @param vector of strings
 * @return vector of doubles
 */
Eigen::VectorXd percentiles_to_probs(
    const std::vector<std::string> &percentiles) {
  Eigen::VectorXd probs(percentiles.size());
  int cur_pct = 0;
  double pct = 0;
  for (size_t i = 0; i < percentiles.size(); ++i) {
    try {
      pct = std::stod(percentiles[i]);
      if (!std::isfinite(pct) || pct < 0.1 || pct > 99.9 || pct < cur_pct)
        throw std::exception();
      cur_pct = pct;
    } catch (const std::exception &e) {
      throw std::invalid_argument(
          "values must be in range (0.1,99.9)"
          ", inclusive, and strictly increasing.");
    }
    probs[i] = pct / 100.0;
  }
  return probs;
}

/**
 * Assemble vector of output column labels, where each column is a statistic.
 * See `get_stats`, (below).
 *
 * @param in vector of percentile values as strings
 * @return vector column labels
 */
std::vector<std::string> get_header(
    const std::vector<std::string> &percentiles) {
  std::vector<std::string> header(percentiles.size() + 7);
  header.at(0) = "Mean";
  header.at(1) = "MCSE";
  header.at(2) = "MaxAbsDev";
  for (size_t i = 0; i < percentiles.size(); ++i) {
    header[i + 3] = percentiles[i] + '%';
  }
  size_t offset = 3 + percentiles.size();
  header.at(offset) = "N_Eff_bulk";
  header.at(offset + 1) = "N_Eff_tail";
  header.at(offset + 2) = "R_hat_bulk";
  header.at(offset + 3) = "R_hat_tail";
  return header;
}

/**
 * Compute per-parameters statistics, consisting of:
 * Mean, MCSE, MaxAbsDev, specified quantile(s),
 * N_eff_bulk, N_eff_tail, R-hat_bulk, R-hat_tail
 * Populate data structure for output, one row per parameter,
 * one column per statistic.
 *
 * Note:  if you change this function, change get_header above
 *
 * @param chains vector of matrices of per-chain draws
 * @param probs vector of probabilities for quantiles
 * @param param_names vector of requested parameter names
 * @param stats matrix of computed statistics
 */
void get_stats(const stan::mcmc::chainset<> &chains,
               const Eigen::VectorXd &probs,
               std::vector<std::string> param_names, Eigen::MatrixXd &stats) {
  stats.setZero();
  size_t i = 0;
  for (std::string name : param_names) {
    stats(i, 0) = chains.mean(name);
    stats(i, 1) = chains.mcse_mean(name);
    stats(i, 2) = chains.max_abs_deviation(name);
    Eigen::VectorXd quantiles = chains.quantiles(name, probs);
    for (int j = 0; j < quantiles.size(); j++)
      stats(i, 3 + j) = quantiles(j);
    size_t offset = 3 + quantiles.size();
    double ess_bulk, ess_tail;
    std::tie(ess_bulk, ess_tail) = chains.split_rank_normalized_ess(name);
    stats(i, offset) = ess_bulk;
    stats(i, offset + 1) = ess_tail;
    double rhat_bulk, rhat_tail;
    std::tie(rhat_bulk, rhat_tail) = chains.split_rank_normalized_rhat(name);
    stats(i, offset + 2) = rhat_bulk;
    stats(i, offset + 3) = rhat_tail;
    i++;
  }
}

/**
 * Output summary header either as fixed-width text columns or in csv format.
 * Indent header by length of longest parameter name.
 *
 * @param in vector of output column labels
 * @param in vector of output column widths
 * @param in length of longest param name
 * @param in output format flag:  true for csv; false for plain text
 * @param in output stream
 */
void write_header(const std::vector<std::string> &header,
                  const Eigen::VectorXi &col_widths, int max_name_length,
                  bool as_csv, std::ostream *out) {
  if (as_csv) {
    *out << "name";
    for (int i = 0; i < header.size(); ++i) {
      *out << "," << header[i];
    }
  } else {
    *out << std::setw(max_name_length + 1) << "";
    for (int i = 0; i < header.size(); ++i) {
      *out << std::setw(col_widths(i)) << header[i];
    }
  }
  *out << std::endl;
}

/**
 * Output statistics for a set of parameters
 * either as fixed-width text columns or in csv format.
 *
 * @param param_names vector of requested parameter names
 * @param stats matrix of computed statistics
 * @param col_widths vector of output column widths
 * @param col_formats vector of output column formats
 * @param max_name_length longest parameter name - (width of 1st output column)
 * @param sig_figs significant digits required
 * @param as_csv flag - true for csv; false for plain text
 * @param out output stream
 */
void write_stats(const std::vector<std::string> &param_names,
                 const Eigen::MatrixXd &stats,
                 const Eigen::VectorXi &col_widths,
                 const Eigen::Matrix<std::ios_base::fmtflags, Eigen::Dynamic, 1>
                     &col_formats,
                 int max_name_length, int sig_figs, bool as_csv,
                 std::ostream *out) {
  bool in_sampler_params = true;
  if (!boost::ends_with(param_names[0], "__")) {
    in_sampler_params = false;
  }
  for (size_t i = 0; i < param_names.size(); ++i) {
    if (as_csv) {
      *out << "\"" << param_names[i] << "\"";
      for (int j = 0; j < stats.cols(); j++) {
        *out << "," << stats(i, j);
      }
    } else {
      if (i > 0 && in_sampler_params
          && !boost::ends_with(param_names[i], "__")) {
        in_sampler_params = false;
        std::cout << std::endl;
      }
      *out << std::setw(max_name_length + 1) << std::left << param_names[i];
      *out << std::right;
      for (int j = 0; j < stats.cols(); j++) {
        std::cout.setf(col_formats(j), std::ios::floatfield);
        *out << std::setprecision(
            compute_precision(stats(i, j), sig_figs, false))
             //                              col_formats(j) ==
             //                              std::ios_base::scientific))
             << std::setw(col_widths(j)) << stats(i, j);
      }
    }
    *out << std::endl;
  }
}

/**
 * Output timing statistics for all chains
 *
 * @param in set of samples from one or more chains
 * @param in metadata
 * @param in warmup times for each chain
 * @param in sampling times for each chain
 * @param in thinning for each chain
 * @param in prefix string - used to output as comments in csv file
 * @param out output stream
 */
void write_timing(const stan::mcmc::chainset<> &chains,
                  const stan::io::stan_csv_metadata &metadata,
                  const Eigen::VectorXd &warmup_times,
                  const Eigen::VectorXd &sampling_times,
                  const Eigen::VectorXi &thin, const std::string &prefix,
                  std::ostream *out) {
  *out << prefix << "Inference for Stan model: " << metadata.model << std::endl
       << prefix << chains.num_chains()
       << " chains: each with iter=" << metadata.num_samples;
  *out << "; warmup=" << metadata.num_warmup;
  *out << "; thin=" << metadata.thin;
  *out << "; " << chains.num_samples() << " iterations saved." << std::endl
       << prefix << std::endl;

  int sig_figs = 2;
  double total_warmup_time = warmup_times.sum();
  double total_sampling_time = sampling_times.sum();

  std::string warmup_unit = "seconds";
  if (total_warmup_time / 3600 > 1) {
    total_warmup_time /= 3600;
    warmup_unit = "hours";
  } else if (total_warmup_time / 60 > 1) {
    total_warmup_time /= 60;
    warmup_unit = "minutes";
  }
  if (chains.num_chains() == 1) {
    *out << prefix << "Warmup took " << std::fixed
         << std::setprecision(
                compute_precision(total_warmup_time, sig_figs, false))
         << total_warmup_time << " " << warmup_unit << std::endl;
  } else {
    *out << prefix << "Warmup took (" << std::fixed
         << std::setprecision(
                compute_precision(warmup_times(0), sig_figs, false))
         << warmup_times(0);
    for (int chain = 1; chain < chains.num_chains(); chain++)
      *out << ", " << std::fixed
           << std::setprecision(
                  compute_precision(warmup_times(chain), sig_figs, false))
           << warmup_times(chain);
    *out << ") seconds, ";
    *out << std::fixed
         << std::setprecision(
                compute_precision(total_warmup_time, sig_figs, false))
         << total_warmup_time << " " << warmup_unit << " total" << std::endl;
  }

  std::string sampling_unit = "seconds";
  if (total_sampling_time / 3600 > 1) {
    total_sampling_time /= 3600;
    sampling_unit = "hours";
  } else if (total_sampling_time / 60 > 1) {
    total_sampling_time /= 60;
    sampling_unit = "minutes";
  }
  if (chains.num_chains() == 1) {
    *out << prefix << "Sampling took " << std::fixed
         << std::setprecision(
                compute_precision(total_sampling_time, sig_figs, false))
         << total_sampling_time << " " << sampling_unit << std::endl;
  } else {
    *out << prefix << "Sampling took (" << std::fixed
         << std::setprecision(
                compute_precision(sampling_times(0), sig_figs, false))
         << sampling_times(0);
    for (int chain = 1; chain < chains.num_chains(); chain++)
      *out << ", " << std::fixed
           << std::setprecision(
                  compute_precision(sampling_times(chain), sig_figs, false))
           << sampling_times(chain);
    *out << ") seconds, ";
    *out << std::fixed
         << std::setprecision(
                compute_precision(total_sampling_time, sig_figs, false))
         << total_sampling_time << " " << sampling_unit << " total"
         << std::endl;
  }
}

/**
 * Output sampler information
 *
 * @param in metadata
 * @param in prefix string - used to output as comments in csv file
 * @param out output stream
 */
void write_sampler_info(const stan::io::stan_csv_metadata &metadata,
                        const std::string &prefix, std::ostream *out) {
  /// Footer output
  *out << prefix << "Samples were drawn using " << metadata.algorithm
       << " with " << metadata.engine << "." << std::endl;
  *out << prefix
       << "For each parameter, N_Eff_bulk and N_Eff_tail measure the "
          "effective sample size "
       << std::endl
       << "for the entire sample and for the "
          "the .05 and .95 tails, respectively, "
       << std::endl;
  *out << prefix
       << "and R_hat_bulk and R_hat_tail measure the potential scale reduction "
       << std::endl
       << "on split chains, (at convergence will be very close to 1)."
       << std::endl;
}

/**
 * Compute and autocorrelation of a specified chain
 * and print to console.
 *
 * <p>Lag is computed based on number of samples:
 * size < 100, lag 1, size < 1000, lag 2, size < 10000 lag 3, etc.

 * @param in set of samples from one or more chains
 * @param in metadata
 * @param in 1-based index of stan csv input file
 * @param in size of longest sampler param name
 */
// autocorrelation report prints to std::out
void autocorrelation(const stan::mcmc::chainset<> &chains,
                     const stan::io::stan_csv_metadata &metadata,
                     int autocorr_idx, int max_name_length) {
  int c = autocorr_idx - 1;
  Eigen::MatrixXd autocorr(chains.num_params(), chains.num_samples());
  for (int i = 0; i < chains.num_params(); ++i) {
    autocorr.row(i) = chains.autocorrelation(c, i);
  }
  std::cout << "Displaying the autocorrelations for chain " << autocorr_idx
            << ":" << std::endl
            << std::endl;

  int n_autocorr = autocorr.row(0).size();
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

  for (int n = 0; n < n_autocorr; ++n) {
    std::cout << std::setw(lag_width) << std::right << n;
    for (int i = 0; i < chains.num_params(); ++i) {
      std::cout << std::setw(max_name_length + 1) << std::right
                << autocorr(i, n);
    }
    std::cout << std::endl;
  }
}

#endif
