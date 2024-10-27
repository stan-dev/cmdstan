#include <cmdstan/return_codes.hpp>
#include <cmdstan/stansummary_helper.hpp>
#include <stan/mcmc/chainset.hpp>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>

using cmdstan::return_codes;

double RHAT_MAX = 1.01499;  // round to 1.01

void diagnose_usage() {
  std::cout << "USAGE:  diagnose <filename 1> [<filename 2> ... <filename N>]"
            << std::endl
            << std::endl;
}

/**
 * Diagnostic checks for NUTS-HMC sampler parameters.
 *
 * @param argc Number of arguments
 * @param argv Arguments
 *
 * @return 0 for success,
 *         non-zero otherwise
 */
int main(int argc, const char *argv[]) {
  if (argc == 1) {
    diagnose_usage();
    return return_codes::OK;
  }

  // Parse any arguments specifying filenames
  std::ifstream ifstream;
  std::vector<std::string> filenames;

  for (int i = 1; i < argc; ++i) {
    ifstream.open(argv[i]);
    if (ifstream.good()) {
      filenames.push_back(argv[i]);
      ifstream.close();
    } else {
      std::cout << "File " << argv[i] << " not found" << std::endl;
    }
  }

  if (!filenames.size()) {
    std::cout << "No valid input files, exiting." << std::endl;
    return return_codes::NOT_OK;
  }

  std::cout << std::fixed << std::setprecision(2);

  std::vector<stan::io::stan_csv> csv_parsed;
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
      csv_parsed.push_back(sample);
    } catch (const std::invalid_argument &e) {
      std::cout << "Cannot parse input csv file: " << filenames[i] << e.what()
                << "." << std::endl;
      return return_codes::NOT_OK;
    }
  }
  stan::mcmc::chainset chains(csv_parsed);
  stan::io::stan_csv_metadata metadata = csv_parsed[0].metadata;
  std::vector<std::string> param_names = csv_parsed[0].header;
  size_t num_params = param_names.size();
  int num_samples = chains.num_samples();
  std::vector<std::string> bad_n_eff_names;
  std::vector<std::string> bad_rhat_names;
  bool has_errors = false;

  for (int i = 0; i < num_params; ++i) {
    if (param_names[i] == std::string("treedepth__")) {
      std::cout << "Checking sampler transitions treedepth." << std::endl;
      int max_limit = metadata.max_depth;
      long n_max = 0;
      Eigen::MatrixXd draws = chains.samples(i);
      Eigen::VectorXd t_samples
          = Eigen::Map<Eigen::VectorXd>(draws.data(), draws.size());
      for (long n = 0; n < t_samples.size(); ++n) {
        if (t_samples(n) >= max_limit) {
          ++n_max;
        }
      }
      if (n_max > 0) {
        has_errors = true;
        double pct = 100 * static_cast<double>(n_max) / num_samples;
        std::cout << n_max << " of " << num_samples << " (" << pct << "%)"
                  << " transitions hit the maximum treedepth limit of "
                  << max_limit << ", or 2^" << max_limit << " leapfrog steps."
                  << std::endl
                  << "Trajectories that are prematurely terminated due to this"
                  << " limit will result in slow exploration." << std::endl
                  << "For optimal performance, increase this limit."
                  << std::endl
                  << std::endl;
      } else {
        std::cout << "Treedepth satisfactory for all transitions." << std::endl
                  << std::endl;
      }
    } else if (param_names[i] == std::string("divergent__")) {
      std::cout << "Checking sampler transitions for divergences." << std::endl;
      int n_divergent = chains.samples(i).sum();
      if (n_divergent > 0) {
        has_errors = true;
        std::cout << n_divergent << " of " << num_samples << " ("
                  << 100 * static_cast<double>(n_divergent) / num_samples
                  << "%) transitions ended with a divergence." << std::endl
                  << "These divergent transitions indicate that HMC is not "
                     "fully able to"
                  << " explore the posterior distribution." << std::endl
                  << "Try increasing adapt delta closer to 1." << std::endl
                  << "If this doesn't remove all"
                  << " divergences, try to reparameterize the model."
                  << std::endl
                  << std::endl;
      } else {
        std::cout << "No divergent transitions found." << std::endl
                  << std::endl;
      }
    } else if (param_names[i] == std::string("energy__")) {
      std::cout << "Checking E-BFMI - sampler transitions HMC potential energy."
                << std::endl;
      Eigen::MatrixXd draws = chains.samples(i);
      Eigen::VectorXd e_samples
          = Eigen::Map<Eigen::VectorXd>(draws.data(), draws.size());
      double delta_e_sq_mean = 0;
      double e_mean = chains.mean(i);
      double e_var = chains.variance(i);
      for (long n = 1; n < e_samples.size(); ++n) {
        double e = e_samples(n);
        double delta_e_sq = (e - e_samples(n - 1)) * (e - e_samples(n - 1));
        double d = delta_e_sq - delta_e_sq_mean;
        delta_e_sq_mean += d / n;
        d = e - e_mean;
      }
      double e_bfmi = delta_e_sq_mean / e_var;
      double e_bfmi_threshold = 0.3;
      if (e_bfmi < e_bfmi_threshold) {
        has_errors = true;
        std::cout << "The E-BFMI, " << e_bfmi << ", is below the nominal"
                  << " threshold of " << e_bfmi_threshold << " which suggests"
                  << " that HMC may have trouble exploring the target"
                  << " distribution." << std::endl
                  << "If possible, try to reparameterize the model."
                  << std::endl
                  << std::endl;
      } else {
        std::cout << "E-BFMI satisfactory." << std::endl << std::endl;
      }
    } else if (param_names[i].find("__") == std::string::npos) {
      auto [ess_bulk, ess_tail] = chains.split_rank_normalized_ess(i);
      double n_eff = ess_bulk < ess_tail ? ess_bulk : ess_tail;
      if (n_eff / num_samples < 0.001)
        bad_n_eff_names.push_back(param_names[i]);

      auto [rhat_bulk, rhat_tail] = chains.split_rank_normalized_rhat(i);
      double split_rhat = rhat_bulk > rhat_tail ? rhat_bulk : rhat_tail;
      if (split_rhat > RHAT_MAX)
        bad_rhat_names.push_back(param_names[i]);
    }
  }
  if (bad_n_eff_names.size() > 0) {
    has_errors = true;
    std::cout << "The following parameters had fewer than 0.001 effective"
              << " draws per transition:" << std::endl;
    std::cout << "  ";
    for (size_t n = 0; n < bad_n_eff_names.size() - 1; ++n)
      std::cout << bad_n_eff_names.at(n) << ", ";
    std::cout << bad_n_eff_names.back() << std::endl;

    std::cout << "Such low values indicate that the effective sample size"
              << " estimators may be biased high and actual performance"
              << " may be substantially lower than quoted." << std::endl
              << std::endl;
  } else {
    std::cout << "Rank-normalized split effective sample size satisfactory "
              << "for all parameters." << std::endl
              << std::endl;
  }

  if (bad_rhat_names.size() > 0) {
    has_errors = true;
    std::cout << "The following parameters had rank-normalized split R-hat "
                 "greater than "
              << RHAT_MAX << ":" << std::endl;
    std::cout << "  ";
    for (size_t n = 0; n < bad_rhat_names.size() - 1; ++n)
      std::cout << bad_rhat_names.at(n) << ", ";
    std::cout << bad_rhat_names.back() << std::endl;

    std::cout << "Such high values indicate incomplete mixing and biased"
              << " estimation." << std::endl
              << "You should consider regularizating your model"
              << " with additional prior information or a more"
              << " effective parameterization." << std::endl
              << std::endl;
  } else {
    std::cout << "Rank-normalized split R-hat values satisfactory "
              << "for all parameters." << std::endl
              << std::endl;
  }
  if (!has_errors)
    std::cout << "Processing complete, no problems detected." << std::endl;
  else
    std::cout << "Processing complete." << std::endl;

  return return_codes::OK;
}
