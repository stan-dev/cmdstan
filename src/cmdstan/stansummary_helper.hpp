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
 * Given a column label, check for array dims, "[n(,n)*]"
 * Return true if no array dimensions found.
 *
 * @param parameter_name column label
 * @return boolean
 */
bool is_scalar(const std::string &parameter_name) {
  return (parameter_name.find("[") == std::string::npos);
}

/**
 * Return parameter name corresponding to column label.
 *
 * @param in column index
 * @return variable name
 */
std::string base_param_name(const std::vector<std::string> &param_names,
                            int index) {
  return param_names[index].substr(0, param_names[index].find("["));
}

/**
 * Return vector of dimensions for container variable.
 * Parameter name at start index contains "[" char.
 * Finds index of final array element and parse its name
 * into a vector dimensions.
 *
 * @param in set of samples from one or more chains
 * @param in column index of first container element
 * @return vector of dimensions
 */
std::vector<int> dimensions(const std::vector<std::string> &param_names,
                            int start_index) {
  std::string name = base_param_name(param_names, start_index);
  int end_index = start_index;
  while (end_index + 1 < param_names.size()) {
    if (base_param_name(param_names, end_index + 1) == name)
      end_index++;
    else
      break;
  }
  std::vector<int> dims;
  int dim;
  std::stringstream ss(
      param_names[end_index].substr(param_names[end_index].find("[")));
  ss.get();  // skip open square bracket
  ss >> dim;
  dims.push_back(dim);
  while (ss.get() == ',') {
    ss >> dim;
    dims.push_back(dim);
  }
  return dims;
}

/**
 * Given a current array coordinates and set of dimensions
 * compute the next coordinate for row-major indexing.
 * Update coordinate and return string of comma separted coords,
 * enclosed by square brackets.
 * Arg arrays must be the same size and next coordinate must not
 * exceed allowed dimension.
 *
 * @param index array of current/next element coords
 * @param dims  array dimensions
 */
void next_index(std::vector<int> &index, const std::vector<int> &dims) {
  if (dims.size() != index.size())
    throw std::domain_error("next_index: size mismatch");
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
 * Given an array of indices, convert to string.
 *
 * @param coords element coords
 * @return coordinates as string
 */
std::string coords_str(const std::vector<int> &coords) {
  std::stringstream ss_coords;
  ss_coords << "[";
  for (size_t i = 0; i < coords.size(); ++i) {
    ss_coords << coords[i];
    if (i < coords.size() - 1)
      ss_coords << ",";
  }
  ss_coords << "]";
  return ss_coords.str();
}

/**
 * Creates array of parameter names where all container parameters
 * are listed in row major order.
 * E.g, ( "x[1,1]", "x[2,1]", "x[1,2]", "x[2,2]" ) becomes
 *      ( "x[1,1]", "x[1,2]", "x[2,1]", "x[2,2]" ) becomes
 *
 *
 * @param vector of strings
 * @return vector of strings
 */
std::vector<std::string> order_param_names_row_major(
    const std::vector<std::string> &param_names) {
  std::vector<std::string> param_names_row_maj(param_names.size());
  int pname_idx = 0;
  while (pname_idx < param_names.size()) {
    if (is_scalar(param_names[pname_idx])) {
      param_names_row_maj[pname_idx] = param_names[pname_idx];
      pname_idx++;
    } else {
      auto basename = base_param_name(param_names, pname_idx);
      auto dims = dimensions(param_names, pname_idx);
      int max = 1;
      for (size_t j = 0; j < dims.size(); j++) {
        max *= dims[j];
      }
      std::vector<int> new_index(dims.size(), 1);
      param_names_row_maj[pname_idx] = basename + coords_str(new_index);
      for (int k = 1; k < max; ++k) {
        next_index(new_index, dims);
        param_names_row_maj[pname_idx + k] = basename + coords_str(new_index);
      }
      pname_idx += max;
    }
  }
  return param_names_row_maj;
}

/**
 * Convert percentiles - string-encoded doubles in range (0,100)
 * to probabilities - double values in range (0, 1).
 *
 * Input values must be in strictly increasing order.
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
      if (!std::isfinite(pct) || pct < 0.0 || pct > 100.0 || pct < cur_pct)
        throw std::exception();
      cur_pct = pct;
    } catch (const std::exception &e) {
      throw std::invalid_argument(
          "values must be in range (0, 100)"
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
  header.at(2) = "StdDev";
  header.at(3) = "MAD";
  size_t offset = 4;
  for (size_t i = 0; i < percentiles.size(); ++i) {
    header[i + offset] = percentiles[i] + '%';
  }
  offset += percentiles.size();
  header.at(offset++) = "ESS_bulk";
  header.at(offset++) = "ESS_tail";
  header.at(offset++) = "R_hat";
  return header;
}

/**
 * Compute per-parameters statistics, consisting of:
 * Mean, MCSE, MAD, specified quantile(s),
 * ESS_bulk, ESS_tail, R-hat_bulk, R-hat_tail
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
void get_stats(const stan::mcmc::chainset &chains, const Eigen::VectorXd &probs,
               const std::vector<std::string> &param_names,
               Eigen::MatrixXd &stats) {
  stats.setZero();
  size_t i = 0;
  for (std::string name : param_names) {
    stats(i, 0) = chains.mean(name);
    stats(i, 1) = chains.mcse_mean(name);
    stats(i, 2) = chains.sd(name);
    stats(i, 3) = chains.max_abs_deviation(name);
    size_t offset = 4;
    Eigen::VectorXd quantiles = chains.quantiles(name, probs);
    for (int j = 0; j < quantiles.size(); j++)
      stats(i, offset++) = quantiles(j);
    auto [ess_bulk, ess_tail] = chains.split_rank_normalized_ess(name);
    stats(i, offset++) = ess_bulk;
    stats(i, offset++) = ess_tail;
    auto [rhat_bulk, rhat_tail] = chains.split_rank_normalized_rhat(name);
    stats(i, offset) = rhat_bulk > rhat_tail ? rhat_bulk : rhat_tail;
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
 * @param reorder_params flag - if true, reorder param names
 * @param out output stream
 */
void write_stats(const std::vector<std::string> &param_names,
                 const Eigen::MatrixXd &stats,
                 const Eigen::VectorXi &col_widths,
                 const Eigen::Matrix<std::ios_base::fmtflags, Eigen::Dynamic, 1>
                     &col_formats,
                 int max_name_length, int sig_figs, bool as_csv,
                 bool reorder_params, std::ostream *out) {
  auto pnames = param_names;
  if (reorder_params) {
    pnames = order_param_names_row_major(param_names);
  }

  bool in_sampler_params = true;
  if (!boost::ends_with(pnames[0], "__")) {
    in_sampler_params = false;
  }
  for (size_t i = 0; i < pnames.size(); ++i) {
    if (as_csv) {
      *out << "\"" << pnames[i] << "\"";
      for (int j = 0; j < stats.cols(); j++) {
        *out << "," << stats(i, j);
      }
    } else {
      if (i > 0 && in_sampler_params && !boost::ends_with(pnames[i], "__")) {
        in_sampler_params = false;
        std::cout << std::endl;
      }
      *out << std::setw(max_name_length + 1) << std::left << pnames[i];
      *out << std::right;
      for (int j = 0; j < stats.cols(); j++) {
        if (boost::ends_with(pnames[i], "__") && pnames[i] != "lp__"
            && j >= stats.cols() - 3)
          continue;  // don't report ESS or Rhat for sampler state
        std::cout.setf(col_formats(j), std::ios::floatfield);
        *out << std::setprecision(compute_precision(
            stats(i, j), sig_figs, col_formats(j) == std::ios_base::scientific))
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
void write_timing(const stan::mcmc::chainset &chains,
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
       << "For each parameter, ESS_bulk and ESS_tail measure the "
          "effective sample size "
       << std::endl
       << "for the entire sample (bulk) and for the "
          "the .05 and .95 tails (tail), "
       << std::endl;
  *out << prefix
       << "and R_hat measures the potential scale reduction on split chains."
       << std::endl
       << "At convergence R_hat will be very close to 1.00." << std::endl;
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
void autocorrelation(const stan::mcmc::chainset &chains,
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
