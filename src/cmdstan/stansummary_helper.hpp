#ifndef CMDSTAN_STANSUMMARY_HELPER_HPP
#define CMDSTAN_STANSUMMARY_HELPER_HPP

#include <stan/mcmc/chains.hpp>
#include <algorithm>
#include <iomanip>
#include <ios>
#include <iostream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

void compute_width_and_precision(double value, int sig_figs, int &width,
                                 int &precision) {
  double abs_value = std::fabs(value);

  if (value == 0) {
    width = sig_figs;
    precision = sig_figs;
  } else if (abs_value >= 1) {
    int int_part = std::ceil(log10(abs_value) + 1e-6);
    width = int_part >= sig_figs ? int_part : sig_figs + 1;
    precision = int_part >= sig_figs ? 0 : sig_figs - int_part;
  } else {
    int frac_part = std::fabs(std::floor(log10(abs_value)));
    width = 1 + frac_part + sig_figs;
    precision = frac_part + sig_figs - 1;
  }

  if (value < 0)
    ++width;
}

int compute_width(double value, int sig_figs) {
  int width;
  int precision;
  compute_width_and_precision(value, sig_figs, width, precision);
  return width;
}

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

int calculate_column_width(const Eigen::VectorXd &x, const std::string &name,
                           const int sig_figs,
                           std::ios_base::fmtflags &format) {
  int padding = 2;

  // Fixed Precision
  size_t fixed_threshold = 8;
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

Eigen::VectorXi calculate_column_widths(
    const Eigen::MatrixXd &values, const std::vector<std::string> &headers,
    const int sig_figs,
    Eigen::Matrix<std::ios_base::fmtflags, Dynamic, 1> &formats) {
  int n = values.cols();
  Eigen::VectorXi column_widths(n);
  formats.resize(n);
  for (int i = 0; i < n; i++) {
    column_widths(i) = calculate_column_width(values.col(i), headers[i],
                                              sig_figs, formats(i));
  }
  return column_widths;
}

bool is_matrix(const std::string &parameter_name) {
  return (parameter_name.find("[") != std::string::npos);
}

std::string base_param_name(stan::mcmc::chains<> &chains, const int index) {
  std::string name = chains.param_name(index);
  return name.substr(0, name.find("["));
}

std::string matrix_index(stan::mcmc::chains<> &chains, const int index) {
  std::string name = chains.param_name(index);
  return name.substr(name.find("["));
}

std::vector<int> dimensions(stan::mcmc::chains<> &chains,
                            const int start_index) {
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

// next 1-based index in row major order
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

// return the flat 0-based index of a column major order matrix based on the
// 1-based index
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

Eigen::VectorXd parse_probs(const std::string &percentiles_spec) {
  std::vector<std::string> percentiles;
  boost::algorithm::split(percentiles, percentiles_spec, boost::is_any_of(", "),
                          boost::token_compress_on);
  Eigen::VectorXd probs(percentiles.size());
  int cur_pct = 0;
  int pct = 0;
  int i = 0;
  for (std::vector<std::string>::iterator it = percentiles.begin();
       it != percentiles.end(); it++, ++i) {
    try {
      pct = std::stoi(*it);
      if (pct < 1 || pct > 99 || pct < cur_pct)
        throw std::exception();
      cur_pct = pct;
    } catch (const std::exception &e) {
      std::stringstream message_stream("");
      message_stream << "invalid config: percentiles must be "
                     << "integers between 0 and 100, increasing, found: "
                     << percentiles_spec << ".";
      throw std::domain_error(message_stream.str());
    }
    probs[i] = pct * 1.0 / 100.0;
  }
  return probs;
}

#endif
