#include <cmdstan/stansummary_helper.hpp>
#include <stan/mcmc/chains.hpp>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>

double RHAT_MAX = 1.05;

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
    return 0;
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
    return 0;
  }

  // Parse specified files
  std::cout << "Processing csv files: " << filenames[0];
  ifstream.open(filenames[0].c_str());

  stan::io::stan_csv stan_csv
      = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
  stan::mcmc::chains<> chains(stan_csv);
  ifstream.close();

  if (filenames.size() > 1)
    std::cout << ", ";
  else
    std::cout << std::endl << std::endl;

  for (std::vector<std::string>::size_type chain = 1; chain < filenames.size();
       ++chain) {
    std::cout << filenames[chain];
    ifstream.open(filenames[chain].c_str());
    stan_csv = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
    chains.add(stan_csv);
    ifstream.close();
    if (chain < filenames.size() - 1)
      std::cout << ", ";
    else
      std::cout << std::endl << std::endl;
  }

  int num_samples = chains.num_samples();
  std::vector<std::string> bad_n_eff_names;
  std::vector<std::string> bad_rhat_names;
  bool has_errors = false;

  for (int i = 0; i < chains.num_params(); ++i) {
    int max_limit = 10;
    if (chains.param_name(i) == std::string("treedepth__")) {
      std::cout << "Checking sampler transitions treedepth." << std::endl;
      int max_limit = stan_csv.metadata.max_depth;
      long n_max = 0;
      Eigen::VectorXd t_samples = chains.samples(i);
      for (long n = 0; n < t_samples.size(); ++n) {
        if (t_samples(n) >= max_limit) {
          ++n_max;
        }
      }
      if (n_max > 0) {
        has_errors = true;
        double pct = 100 * static_cast<double>(n_max) / num_samples;
        std::cout << n_max << " of " << num_samples << " ("
                  << std::setprecision(2) << pct << "%)"
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
    } else if (chains.param_name(i) == std::string("divergent__")) {
      std::cout << "Checking sampler transitions for divergences." << std::endl;
      int n_divergent = chains.samples(i).sum();
      if (n_divergent > 0) {
        has_errors = true;
        std::cout << n_divergent << " of " << num_samples << " ("
                  << std::setprecision(2)
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
    } else if (chains.param_name(i) == std::string("energy__")) {
      std::cout << "Checking E-BFMI - sampler transitions HMC potential energy."
                << std::endl;
      Eigen::VectorXd e_samples = chains.samples(i);
      double delta_e_sq_mean = 0;
      double e_mean = 0;
      double e_var = 0;
      e_mean += e_samples(0);
      e_var += e_samples(0) * (e_samples(0) - e_mean);
      for (long n = 1; n < e_samples.size(); ++n) {
        double e = e_samples(n);
        double delta_e_sq = (e - e_samples(n - 1)) * (e - e_samples(n - 1));
        double d = delta_e_sq - delta_e_sq_mean;
        delta_e_sq_mean += d / n;
        d = e - e_mean;
        e_mean += d / (n + 1);
        e_var += d * (e - e_mean);
      }

      e_var /= static_cast<double>(e_samples.size() - 1);
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
    } else if (chains.param_name(i).find("__") == std::string::npos) {
      double n_eff = chains.effective_sample_size(i);
      if (n_eff / num_samples < 0.001)
        bad_n_eff_names.push_back(chains.param_name(i));

      double split_rhat = chains.split_potential_scale_reduction(i);
      if (split_rhat > RHAT_MAX)
        bad_rhat_names.push_back(chains.param_name(i));
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
    std::cout << "Effective sample size satisfactory." << std::endl
              << std::endl;
  }

  if (bad_rhat_names.size() > 0) {
    has_errors = true;
    std::cout << "The following parameters had split R-hat greater than "
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
    std::cout << "Split R-hat values satisfactory all parameters." << std::endl
              << std::endl;
  }
  if (!has_errors)
    std::cout << "Processing complete, no problems detected." << std::endl;
  else
    std::cout << "Processing complete." << std::endl;

  return 0;
}
