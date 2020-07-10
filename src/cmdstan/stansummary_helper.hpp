#ifndef CMDSTAN_STANSUMMARY_HELPER_HPP
#define CMDSTAN_STANSUMMARY_HELPER_HPP

#include <stan/mcmc/chains.hpp>
#include <algorithm>
#include <fstream>
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
                           int sig_figs, std::ios_base::fmtflags &format) {
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
    int sig_figs, Eigen::Matrix<std::ios_base::fmtflags, Dynamic, 1> &formats) {
  int n = values.cols();
  Eigen::VectorXi column_widths(n);
  formats.resize(n);
  for (int i = 0; i < n; i++) {
    column_widths(i) = calculate_column_width(values.col(i), headers[i],
                                              sig_figs, formats(i));
  }
  return column_widths;
}

bool is_container(const std::string &parameter_name) {
  return (parameter_name.find("[") != std::string::npos);
}

std::string base_param_name(const stan::mcmc::chains<> &chains, int index) {
  std::string name = chains.param_name(index);
  return name.substr(0, name.find("["));
}

std::string matrix_index(const stan::mcmc::chains<> &chains, int index) {
  std::string name = chains.param_name(index);
  return name.substr(name.find("["));
}

std::vector<int> dimensions(const stan::mcmc::chains<> &chains,
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

Eigen::VectorXd percentiles_to_probs(
    const std::vector<std::string> &percentiles) {
  Eigen::VectorXd probs(percentiles.size());
  int cur_pct = 0;
  int pct = 0;
  int i = 0;
  for (size_t i = 0; i < percentiles.size(); ++i) {
    try {
      pct = std::stoi(percentiles[i]);
      if (pct < 1 || pct > 99 || pct < cur_pct)
        throw std::exception();
      cur_pct = pct;
    } catch (const std::exception &e) {
      std::stringstream message_stream("");
      message_stream << "invalid config: percentiles must be "
                     << "integers between 0 and 100, increasing, found: "
                     << percentiles[i] << ".";
      throw std::domain_error(message_stream.str());
    }
    probs[i] = pct * 1.0 / 100.0;
  }
  return probs;
}

stan::mcmc::chains<> parse_csv_files(const std::vector<std::string> &filenames,
                                     stan::io::stan_csv_metadata &metadata,
                                     Eigen::VectorXd &warmup_times,
                                     Eigen::VectorXd &sampling_times,
                                     Eigen::VectorXi &thin, std::ostream *out) {
  // instantiate stan::mcmc::chains object by parsing first file
  std::ifstream ifstream;
  ifstream.open(filenames[0].c_str());
  stan::io::stan_csv stan_csv = stan::io::stan_csv_reader::parse(ifstream, out);
  warmup_times(0) = stan_csv.timing.warmup;
  sampling_times(0) = stan_csv.timing.sampling;
  stan::mcmc::chains<> chains(stan_csv);
  ifstream.close();
  thin(0) = stan_csv.metadata.thin;
  metadata = stan_csv.metadata;

  // parse rest of input files, add to chains
  for (std::vector<std::string>::size_type chain = 1; chain < filenames.size();
       chain++) {
    ifstream.open(filenames[chain].c_str());
    stan_csv = stan::io::stan_csv_reader::parse(ifstream, out);
    chains.add(stan_csv);
    ifstream.close();
    thin(chain) = stan_csv.metadata.thin;
    warmup_times(chain) = stan_csv.timing.warmup;
    sampling_times(chain) = stan_csv.timing.sampling;
  }
  return chains;
}

std::vector<std::string> get_sampler_params_header(
    const std::vector<std::string> &percentiles) {
  // Mean, StdDev, ... percentiles ...
  std::vector<std::string> sampler_params_header(percentiles.size() + 2);
  sampler_params_header.at(0) = "Mean";
  sampler_params_header.at(1) = "StdDev";
  size_t offset = 2;
  for (size_t i = 0; i < percentiles.size(); ++i) {
    sampler_params_header[i + offset] = percentiles[i] + '%';
  }
  return sampler_params_header;
}

std::vector<std::string> get_model_params_header(
    const std::vector<std::string> &percentiles) {
  // Mean, MCSE,  StdDev, ... percentiles ..., N_eff, N_eff/s, R_hat
  std::vector<std::string> model_params_header(percentiles.size() + 6);
  model_params_header.at(0) = "Mean";
  model_params_header.at(1) = "MCSE";
  model_params_header.at(2) = "StdDev";
  size_t offset = 3;
  for (size_t i = 0; i < percentiles.size(); ++i) {
    model_params_header[i + offset] = percentiles[i] + '%';
  }
  offset += percentiles.size();
  model_params_header.at(offset) = "N_Eff";
  model_params_header.at(offset + 1) = "N_Eff/s";
  model_params_header.at(offset + 2) = "R_hat";
  return model_params_header;
}

void sampler_params_stats(const stan::mcmc::chains<> &chains,
                          const Eigen::VectorXd &probs,
                          int sampler_params_start_col,
                          Eigen::MatrixXd &sampler_params) {
  sampler_params.setZero();
  for (int i = 0; i < sampler_params.rows(); ++i) {
    int i_offset = i + sampler_params_start_col;
    sampler_params(i, 0) = chains.mean(i_offset);
    sampler_params(i, 1) = chains.sd(i_offset);
    Eigen::VectorXd quantiles = chains.quantiles(i_offset, probs);
    for (int j = 0; j < probs.size(); j++)
      sampler_params(i, 2 + j) = quantiles(j);
  }
}

void model_params_stats(const stan::mcmc::chains<> &chains,
                        const Eigen::VectorXd &warmup_times,
                        const Eigen::VectorXd &sampling_times,
                        const Eigen::VectorXd &probs,
                        int model_params_start_col,
                        Eigen::MatrixXd &model_params) {
  model_params.setZero();
  double total_warmup_time = warmup_times.sum();
  double total_sampling_time = sampling_times.sum();

  // Joint log prob lp__
  model_params(0, 0) = chains.mean(0);
  double lp_sd = chains.sd(0);
  double lp_n_eff = chains.effective_sample_size(0);
  model_params(0, 1) = lp_sd / sqrt(lp_n_eff);
  model_params(0, 2) = lp_sd;
  Eigen::VectorXd quantiles = chains.quantiles(0, probs);
  for (int j = 0; j < probs.size(); j++)
    model_params(0, 3 + j) = quantiles(j);
  model_params(0, probs.size() + 3) = lp_n_eff;
  model_params(0, probs.size() + 4) = lp_n_eff / total_sampling_time;
  model_params(0, probs.size() + 5) = chains.split_potential_scale_reduction(0);

  // Model parameters
  for (int i = model_params_start_col; i < chains.num_params(); ++i) {
    int row_offset = i - model_params_start_col + 1;
    double sd = chains.sd(i);
    double n_eff = chains.effective_sample_size(i);
    model_params(row_offset, 0) = chains.mean(i);
    model_params(row_offset, 1) = sd / sqrt(n_eff);
    model_params(row_offset, 2) = sd;
    Eigen::VectorXd quantiles = chains.quantiles(i, probs);
    for (int j = 0; j < probs.size(); j++)
      model_params(row_offset, 3 + j) = quantiles(j);
    model_params(row_offset, probs.size() + 3) = n_eff;
    model_params(row_offset, probs.size() + 4) = n_eff / total_sampling_time;
    model_params(row_offset, probs.size() + 5)
        = chains.split_potential_scale_reduction(i);
  }
}

void sampler_params_summary(
    const stan::mcmc::chains<> &chains, const Eigen::MatrixXd &sampler_params,
    const std::vector<std::string> &sampler_params_header,
    int sampler_params_start_col, int max_name_length, int sig_figs,
    std::ostream *out) {
  Eigen::VectorXi sampler_params_column_sig_figs(sampler_params_header.size());
  Eigen::VectorXi sampler_params_column_widths(sampler_params_header.size());
  Eigen::Matrix<std::ios_base::fmtflags, Eigen::Dynamic, 1>
      sampler_params_formats(sampler_params_header.size());
  sampler_params_column_widths = calculate_column_widths(
      sampler_params, sampler_params_header, sig_figs, sampler_params_formats);

  *out << std::setw(max_name_length + 1) << "";
  for (int i = 0; i < sampler_params.cols(); ++i) {
    *out << std::setw(sampler_params_column_widths(i))
         << sampler_params_header[i];
  }
  *out << std::endl;

  for (int i = 0; i < sampler_params.rows(); ++i) {
    int row_offset = i + sampler_params_start_col;
    *out << std::setw(max_name_length + 1) << std::left
         << chains.param_name(row_offset);
    *out << std::right;
    for (int j = 0; j < sampler_params.cols(); j++) {
      std::cout.setf(sampler_params_formats(j), std::ios::floatfield);
      *out << std::setprecision(compute_precision(
          sampler_params(i, j), sig_figs,
          sampler_params_formats(j) == std::ios_base::scientific))
           << std::setw(sampler_params_column_widths(j))
           << sampler_params(i, j);
    }
    *out << std::endl;
  }
}

void model_params_summary(const stan::mcmc::chains<> &chains,
                          const Eigen::MatrixXd &model_params,
                          const std::vector<std::string> &model_params_header,
                          int model_params_start_col, int max_name_length,
                          int sig_figs, std::ostream *out) {
  Eigen::VectorXi model_params_column_sig_figs(model_params_header.size());
  Eigen::VectorXi model_params_column_widths(model_params_header.size());
  Eigen::Matrix<std::ios_base::fmtflags, Eigen::Dynamic, 1>
      model_params_formats(model_params_header.size());
  model_params_column_widths = calculate_column_widths(
      model_params, model_params_header, sig_figs, model_params_formats);

  *out << std::setw(max_name_length + 1) << "";
  for (int i = 0; i < model_params_header.size(); ++i) {
    *out << std::setw(model_params_column_widths(i)) << model_params_header[i];
  }
  *out << std::endl;

  // Joint log prob lp__
  *out << std::setw(max_name_length + 1) << std::left << chains.param_name(0);
  *out << std::right;
  for (int j = 0; j < model_params.cols(); j++) {
    std::cout.setf(model_params_formats(j), std::ios::floatfield);
    *out << std::setprecision(
        compute_precision(model_params(0, j), sig_figs,
                          model_params_formats(j) == std::ios_base::scientific))
         << std::setw(model_params_column_widths(j)) << model_params(0, j);
  }
  *out << std::endl;

  // Individual model parameters
  int num_sampler_params = model_params_start_col - 1;
  for (int i = 0; i < model_params.rows() - 1; ++i) {
    int i_offset = i + model_params_start_col;
    if (!is_container(chains.param_name(i_offset))) {
      *out << std::setw(max_name_length + 1) << std::left
           << chains.param_name(i_offset);
      *out << std::right;
      for (int j = 0; j < model_params.cols(); j++) {
        std::cout.setf(model_params_formats(j), std::ios::floatfield);
        *out << std::setprecision(compute_precision(
            model_params(i, j), sig_figs,
            model_params_formats(j) == std::ios_base::scientific))
             << std::setw(model_params_column_widths(j)) << model_params(i, j);
      }
      *out << std::endl;
    } else {
      // container object columns in csv are last-index-major order
      // output as first-index-major order
      std::vector<int> dims = dimensions(chains, i_offset);
      std::vector<int> index(dims.size(), 1);
      int max = 1;
      for (size_t j = 0; j < dims.size(); j++)
        max *= dims[j];
      for (int k = 0; k < max; k++) {
        int row_maj_index = i_offset + matrix_index(index, dims);
        *out << std::setw(max_name_length + 1) << std::left
             << chains.param_name(row_maj_index);
        *out << std::right;
        for (int j = 0; j < model_params_header.size(); j++) {
          std::cout.setf(model_params_formats(j), std::ios::floatfield);
          *out << std::setprecision(compute_precision(
              model_params(row_maj_index - num_sampler_params, j), sig_figs,
              model_params_formats(j) == std::ios_base::scientific))
               << std::setw(model_params_column_widths(j))
               << model_params(row_maj_index - num_sampler_params, j);
        }
        *out << std::endl;
        if (k < max - 1)
          next_index(index, dims);
      }
      i += max - 1;
    }
  }
}

void timing_summary(const stan::mcmc::chains<> &chains,
                    const stan::io::stan_csv_metadata &metadata,
                    const Eigen::VectorXd &warmup_times,
                    const Eigen::VectorXd &sampling_times,
                    const Eigen::VectorXi &thin, const std::string &prefix,
                    std::ostream *out) {
  *out << prefix << "Inference for Stan model: " << metadata.model << std::endl
       << prefix << chains.num_chains() << " chains: each with iter=("
       << chains.num_kept_samples(0);
  for (int chain = 1; chain < chains.num_chains(); chain++)
    *out << "," << chains.num_kept_samples(chain);
  *out << ")";
  *out << "; warmup=(" << chains.warmup(0);
  for (int chain = 1; chain < chains.num_chains(); chain++)
    *out << "," << chains.warmup(chain);
  *out << ")";
  *out << "; thin=(" << thin(0);
  for (int chain = 1; chain < chains.num_chains(); chain++)
    *out << "," << thin(chain);
  *out << ")";
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
  *out << prefix << std::endl;
}

void sampler_summary(const stan::io::stan_csv_metadata &metadata,
                     const std::string &prefix, std::ostream *out) {
  /// Footer output
  *out << prefix << std::endl;
  *out << prefix << "Samples were drawn using " << metadata.algorithm
       << " with " << metadata.engine << "." << std::endl;
  *out << prefix
       << "For each parameter, N_Eff is a crude measure of effective "
          "sample size,"
       << std::endl;
  *out << prefix
       << "and R_hat is the potential scale reduction factor on split "
          "chains (at "
       << std::endl;
  *out << prefix << "convergence, R_hat=1)." << std::endl;
  *out << prefix << std::endl;
}

// autocorrelation report prints to std::out
void autocorrelation(const stan::mcmc::chains<> &chains,
                     const stan::io::stan_csv_metadata &metadata,
                     int autocorr_idx, int max_name_length) {
  int c = autocorr_idx;
  if (c < 0 || c >= chains.num_chains()) {
    std::cout << "Bad chain index " << c
              << ", aborting autocorrelation display." << std::endl;
  } else {
    Eigen::MatrixXd autocorr(chains.num_params(), chains.num_samples(c));
    for (int i = 0; i < chains.num_params(); ++i) {
      autocorr.row(i) = chains.autocorrelation(c, i);
    }
    std::cout << "Displaying the autocorrelations for chain " << c << ":"
              << std::endl
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
}
#endif
