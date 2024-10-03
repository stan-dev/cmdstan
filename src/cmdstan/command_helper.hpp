#ifndef CMDSTAN_HELPER_HPP
#define CMDSTAN_HELPER_HPP

#include <cmdstan/arguments/argument_parser.hpp>
#include <cmdstan/arguments/arg_sample.hpp>
#include <cmdstan/file.hpp>
#include <stan/callbacks/unique_stream_writer.hpp>
#include <stan/callbacks/json_writer.hpp>
#include <stan/callbacks/writer.hpp>
#include <stan/io/dump.hpp>
#include <stan/io/empty_var_context.hpp>
#include <stan/io/json/json_data.hpp>
#include <stan/io/stan_csv_reader.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/model/log_prob_grad.hpp>
#include <stan/model/model_base.hpp>
#include <stan/services/sample/standalone_gqs.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include <rapidjson/document.h>

namespace cmdstan {
namespace internal {
/**
 * Base of helper function for getting arguments
 * @param x A pointer to an argument in the argument pointer list
 */
template <typename T>
inline constexpr auto get_arg_pointer(T &&x) {
  return x;
}

/**
 * Given a pointer to a list of argument pointers, extract the named argument
 * from the list.
 * @tparam List A pointer to a list that has a valid arg(const char*) method
 * @tparam Args A parameter pack of const char*
 * @param arg_list The list argument to access the arg from
 * @param arg1 The name of the first argument to extract
 * @param args An optional pack of named arguments to access from the first arg
 */
template <typename List, typename... Args>
inline constexpr auto get_arg_pointer(List &&arg_list, const char *arg1,
                                      Args &&... args) {
  return get_arg_pointer(arg_list->arg(arg1), args...);
}

}  // namespace internal

/**
 * Given a list of argument pointers, extract the named argument from the list.
 * @tparam List An list argument that has a valid arg(const char*) method
 * @tparam Args A parameter pack of const char*
 * @param arg_list The list argument to access the arg from
 * @param arg1 The name of the first argument to extract
 * @param args An optional pack of named arguments to access from the first arg
 */
template <typename List, typename... Args>
inline constexpr auto get_arg(List &&arg_list, const char *arg1,
                              Args &&... args) {
  return internal::get_arg_pointer(arg_list.arg(arg1), args...);
}

/**
 * Given an argument return its value. Because all of the elements in
 * our list of command line arguments is an `argument` class with no
 * `value()` method, we must give the function the type of the argument class we
 * want to access.
 * @tparam caster The type to cast the `argument` class in the list to
 * @tparam Arg An object that inherits from `argument`
 * @param argument holds the argument to access
 * @param arg_name The name of the argument to access
 */
template <typename caster, typename Arg>
inline constexpr auto get_arg_val(Arg &&argument, const char *arg_name) {
  auto *arg = argument.arg(arg_name);
  if (arg) {
    return dynamic_cast<std::decay_t<caster> *>(arg)->value();
  } else {
    throw std::invalid_argument(std::string("Unable to find: ") + arg_name);
  }
}

/**
 * Given a list of arguments, index into the args and return the value held
 * by the underlying element in the list. Because all of the elements in
 * our list of command line arguments is an `argument` class with no
 * `value()` method, we must give the function the type of the argument class we
 * want to access.
 * @tparam caster The type to cast the `argument` class in the list to
 * @tparam List A pointer or object that inherits from `argument`
 * @param arg_list holds the arguments to access
 * @param args A parameter pack of names of arguments to index into
 */
template <typename caster, typename List, typename... Args>
inline constexpr auto get_arg_val(List &&arg_list, Args &&... args) {
  auto *x = get_arg(arg_list, args...);
  if (x != nullptr) {
    return dynamic_cast<std::decay_t<caster> *>(x)->value();
  } else {
    throw std::invalid_argument("encountered nullptr");
  }
}

using shared_context_ptr = std::shared_ptr<stan::io::var_context>;
/**
 * Given the name of a file, return a shared pointer holding the data contents.
 * @param file A system file to read from
 */
inline shared_context_ptr get_var_context(const std::string &file) {
  if (file.empty()) {
    return std::make_shared<stan::io::empty_var_context>();
  }
  std::ifstream stream = file::safe_open(file);
  if (file::get_suffix(file) == ".json") {
    stan::json::json_data var_context(stream);
    return std::make_shared<stan::json::json_data>(var_context);
  }
  std::cerr
      << "Warning: file '" << file
      << "' is being read as an 'RDump' file.\n"
         "\tThis format is deprecated and will not receive new features.\n"
         "\tConsider saving your data in JSON format instead."
      << std::endl;
  stan::io::dump var_context(stream);
  return std::make_shared<stan::io::dump>(var_context);
}

using context_vector = std::vector<shared_context_ptr>;
/**
 * Make a vector of shared pointers to contexts.
 * @param file The name of the file. For multi-chain we will attempt to find
 *  {file_name}_1{file_ending} and if that fails try to use the named file as
 *  the data for each chain.
 * @param num_chains The number of chains to run
 * @return a std vector of shared pointers to var contexts
 */
context_vector get_vec_var_context(const std::string &file, size_t num_chains,
                                   unsigned int id) {
  using stan::io::var_context;
  if (num_chains == 1) {
    return context_vector(1, get_var_context(file));
  }
  auto make_context = [](auto &&file, auto &&stream,
                         auto &&file_ending) -> shared_context_ptr {
    if (file_ending == ".json") {
      using stan::json::json_data;
      return std::make_shared<json_data>(json_data(stream));
    } else if (file_ending == ".R") {
      using stan::io::dump;
      return std::make_shared<stan::io::dump>(dump(stream));
    } else {
      std::stringstream msg;
      msg << "file ending of " << file_ending << " is not supported by cmdstan";
      throw std::invalid_argument(msg.str());
      using stan::io::dump;
      return std::make_shared<dump>(dump(stream));
    }
  };
  // use default for all chain inits
  if (file.empty()) {
    return context_vector(num_chains,
                          std::make_shared<stan::io::empty_var_context>());
  } else {
    size_t file_marker_pos = file.find_last_of(".");
    if (file_marker_pos > file.size()) {
      std::stringstream msg;
      msg << "Found: \"" << file
          << "\" but user specified files must end in .json or .R";
      throw std::invalid_argument(msg.str());
    }
    std::string file_name = file.substr(0, file_marker_pos);
    std::string file_ending = file.substr(file_marker_pos, file.size());
    if (file_ending != ".json" && file_ending != ".R") {
      std::stringstream msg;
      msg << "file ending of " << file_ending << " is not supported by cmdstan";
      throw std::invalid_argument(msg.str());
    }
    if (file_ending != ".json") {
      std::cerr
          << "Warning: file '" << file
          << "' is being read as an 'RDump' file.\n"
             "\tThis format is deprecated and will not receive new features.\n"
             "\tConsider saving your data in JSON format instead."
          << std::endl;
    }

    auto filenames
        = file::make_filenames(file_name, "", file_ending, num_chains, id);
    auto &file_1 = filenames[0];
    std::fstream stream_1(file_1.c_str(), std::fstream::in);
    // if file_1 exists we'll assume num_chains of these files exist
    if (stream_1.rdstate() & std::ifstream::failbit) {
      // if that fails we will try to find a base file
      std::fstream stream(file.c_str(), std::fstream::in);
      if (stream.rdstate() & std::ifstream::failbit) {
        std::string file_name_err
            = std::string("\"" + file_1 + "\" and base file \"" + file + "\"");
        std::stringstream msg;
        msg << "Searching for  \"" << file_name_err << std::endl;
        msg << "Can't open either of specified files," << file_name_err
            << std::endl;
        throw std::invalid_argument(msg.str());
      } else {
        return context_vector(num_chains,
                              make_context(file, stream, file_ending));
      }
    } else {
      // If we found file_1 then we'll assume file_{1...N} exists
      context_vector ret;
      ret.reserve(num_chains);
      ret.push_back(make_context(file_1, stream_1, file_ending));
      for (size_t i = 1; i < num_chains; ++i) {
        auto &file_i = filenames[i];
        std::fstream stream_i(file_i.c_str(), std::fstream::in);
        // If any stream fails here something went wrong with file names
        if (stream_i.rdstate() & std::ifstream::failbit) {
          std::string file_name_err = std::string(
              "\"" + file_1 + "\" but cannot open \"" + file_i + "\"");
          std::stringstream msg;
          msg << "Found " << file_name_err << std::endl;
          throw std::invalid_argument(msg.str());
        }
        ret.push_back(make_context(file_i, stream_i, file_ending));
      }
      return ret;
    }
  }
  // This should not happen
  std::cerr
      << "Warning: file '" << file
      << "' is being read as an 'RDump' file.\n"
         "\tThis format is deprecated and will not receive new features.\n"
         "\tConsider saving your data in JSON format instead."
      << std::endl;
  using stan::io::dump;
  std::fstream stream(file.c_str(), std::fstream::in);
  return context_vector(num_chains, std::make_shared<dump>(dump(stream)));
}

/**
 * Get model constrained parameters names.
 * Throws error if model doesn't have any parameters.
 * @param model instantiated model
 * @return vector of constrained parameter names
 */
std::vector<std::string> get_constrained_param_names(
    const stan::model::model_base &model) {
  std::vector<std::string> param_names;
  model.constrained_param_names(param_names, false, false);
  if (param_names.size() < 1) {
    std::stringstream msg;
    msg << "Model " << model.model_name()
        << " has no parameters, nothing to estimate." << std::endl;
    throw std::invalid_argument(msg.str());
  }
  return param_names;
}

/**
 * Parse a StanCSV output file and identify the rows and columns in the
 * data table which contain the fitted estimates of the model parameters.
 * Throws an exception if the StanCSV parser cannot process the file.
 *
 * @param fname name of file which exists and has read perms
 * @param model instantiated model
 * @param param_names
 * @param fitted_params struct which contains CSV header and data rows
 * @param col_offset first column of model outputs in the data table
 * @param num_rows total data table rows
 * @param num_cols total data table columns with parameter variable values
 */
void parse_stan_csv(const std::string &fname,
                    const stan::model::model_base &model,
                    const std::vector<std::string> &param_names,
                    stan::io::stan_csv &fitted_params, size_t &col_offset,
                    size_t &num_rows, size_t &num_cols) {
  std::stringstream msg;
  // parse CSV contents
  std::ifstream stream = file::safe_open(fname);
  stan::io::stan_csv_reader::read_metadata(stream, fitted_params.metadata);
  if (!stan::io::stan_csv_reader::read_header(stream, fitted_params.header,
                                              false)) {
    msg << "Error reading fitted param names from sample csv file \"" << fname
        << "\"" << std::endl;
    throw std::invalid_argument(msg.str());
  }
  stan::io::stan_csv_reader::read_adaptation(stream, fitted_params.adaptation);
  fitted_params.timing.warmup = 0;
  fitted_params.timing.sampling = 0;
  stan::io::stan_csv_reader::read_samples(stream, fitted_params.samples,
                                          fitted_params.timing);
  stream.close();
  // compute offset, size of parameters block
  col_offset = 0;
  for (auto col_name : fitted_params.header) {
    if (boost::algorithm::ends_with(col_name, "__")) {
      col_offset++;
    } else {
      break;
    }
  }
  num_cols = param_names.size();
  num_rows = fitted_params.samples.rows();
  if (num_cols + col_offset > fitted_params.header.size()) {
    msg << "Mismatch between model and fitted_parameters csv file \"" << fname
        << "\"" << std::endl;
    throw std::invalid_argument(msg.str());
  }
  for (size_t i = 0; i < num_cols; ++i) {
    if (param_names[i].compare(fitted_params.header[i + col_offset]) != 0) {
      msg << "Mismatch between model and fitted_parameters csv file \"" << fname
          << "\"" << std::endl;
      throw std::invalid_argument(msg.str());
    }
  }
}

/**
 * Apply model's "unconstrain_array" method to a vector of fitted parameters,
 * return corresponding unconstrained params.
 * Throws an exception if the unconstraining transform fails.
 *
 * @param model instantiated model
 * @param cparams vector of constrained param values
 * @return a std vector of unconstrained parameter values
 */
std::vector<double> unconstrain_params(const stan::model::model_base &model,
                                       const std::vector<double> &cparams) {
  std::stringstream msg;
  size_t num_uparams = model.num_params_r();
  std::vector<double> uparams(num_uparams);
  try {
    model.unconstrain_array(cparams, uparams, &msg);
  } catch (const std::exception &e) {
    std::stringstream msg2;
    msg2 << e.what() << std::endl;
    msg2 << "Bad or missing parameter values, cannot unconstrain.";
    if (msg.str().length() > 0)
      msg2 << "\n\t" << msg.str();
    msg2 << std::endl;
    throw std::invalid_argument(msg2.str());
  }
  return uparams;
}

/**
 * Given an instantiated model and parsed StanCSV output file,
 * apply model's "unconstrain_array" to the fitted parameters.
 * Returns a vector of vectors of parameters on the unconstrained scale.
 * Throws an exception if the unconstraining transform fails.
 *
 * @param model instantiated model
 * @param cparams vector of constrained param values
 * @return a vector of std vectors of unconstrained parameter values
 */
std::vector<std::vector<double>> unconstrain_params_csv(
    const stan::model::model_base &model, stan::io::stan_csv &fitted_params,
    size_t &col_offset, size_t &num_rows, size_t &num_cols) {
  std::vector<std::vector<double>> result;
  for (size_t i = 0; i < num_rows; ++i) {
    std::vector<double> uparams = unconstrain_params(
        model, stan::math::to_array_1d(
                   fitted_params.samples.block(i, col_offset, 1, num_cols)));
    result.emplace_back(std::move(uparams));
  }
  return result;
}

/**
 * Get constrained parameter values from JSON or Rdump file and
 * return corresponding unconstrained params.
 * Throws an exception if the unconstraining transform fails.
 *
 * @param fname name of file which exists and has read perms
 * @param model Stan model
 * @return vector of vectors of parameter estimates
 */
std::vector<double> unconstrain_params_var_context(
    const std::string &fname, const stan::model::model_base &model) {
  std::stringstream msg;
  std::shared_ptr<stan::io::var_context> cpars_context = get_var_context(fname);
  size_t num_upars = model.num_params_r();
  std::vector<double> params(num_upars);
  std::vector<int> dummy_params_i;
  try {
    model.transform_inits((*cpars_context), dummy_params_i, params, &msg);
  } catch (const std::exception &e) {
    std::stringstream msg2;
    msg2 << e.what() << std::endl;
    msg2 << "Bad or missing parameter values, cannot unconstrain.";
    if (msg.str().length() > 0)
      msg2 << "\n\t" << msg.str();
    msg2 << std::endl;
    throw std::invalid_argument(msg2.str());
  }
  return params;
}

/**
 * Parse a StanCSV output file created by the optimizer and return a vector
 * containing the estimates of the model parameters on the unconstrained scale.
 * Throws an exception if it cannot parse the CSV file.
 *
 * @param fname name of file which exists and has read perms
 * @param model Stan model
 * @return Eigen vector of unconstrained parameter estimates
 */
Eigen::VectorXd get_laplace_mode_csv(const std::string &fname,
                                     const stan::model::model_base &model) {
  std::stringstream msg;
  std::vector<std::string> cparam_names;
  model.constrained_param_names(cparam_names);

  // parse CSV file: header comments (config), header row, single data row
  std::string line;
  bool is_optimization = false;
  std::ifstream in = file::safe_open(fname);
  while (in.peek() == '#') {
    std::getline(in, line);
    if (boost::contains(line, "method = optimize"))
      is_optimization = true;
  }
  std::getline(in, line);
  std::vector<std::string> names;
  boost::algorithm::split(names, line, boost::is_any_of(","),
                          boost::token_compress_on);
  std::getline(in, line);
  std::vector<std::string> values;
  boost::algorithm::split(values, line, boost::is_any_of(","),
                          boost::token_compress_on);
  in.close();
  // validate
  if (!is_optimization) {
    msg << "CSV file is not output from Stan optimization" << std::endl;
    throw std::invalid_argument(msg.str());
  }
  // columns: algorithm outputs ending in "__", params, txparms, and gq vars
  size_t col_offset = 0;
  for (auto name : names) {
    if (boost::algorithm::ends_with(name, "__")) {
      col_offset++;
    } else {
      break;
    }
  }
  if (names.size() - col_offset < cparam_names.size()
      || values.size() - col_offset < cparam_names.size()
      || names.size() != values.size()) {
    msg << "CSV file is incomplete, expecting at least "
        << (cparam_names.size() + 1) << " columns." << std::endl;
    throw std::invalid_argument(msg.str());
  }
  // extract constrained parameter values
  std::vector<double> cparams(cparam_names.size());
  for (size_t i = 0; i < cparam_names.size(); ++i) {
    if (cparam_names[i].compare(names[i + col_offset]) != 0) {
      msg << "Mismatch between model params and StanCSV file \"" << fname
          << "\",  expecting param \"" << cparam_names[i] << "\", found \""
          << names[i + col_offset] << "\"" << std::endl;
      throw std::invalid_argument(msg.str());
    }
    try {
      cparams[i] = std::stod(values[i + col_offset]);
    } catch (const std::exception &e) {
      msg << "Error parsing CSV file, bad value for " << cparam_names[i]
          << std::endl;
      throw std::invalid_argument(msg.str());
    }
  }
  std::vector<double> uparams = unconstrain_params(model, cparams);
  Eigen::VectorXd result(uparams.size());
  for (size_t i = 0; i < uparams.size(); ++i) {
    result(i) = uparams[i];
  }
  return result;
}

/**
 * Parse contents of file containing estimate of parameter modes.
 *
 * @param fname name of file which exists and has read perms
 * @param model Stan model
 * @return Eigen vector of parameter estimates
 */
Eigen::VectorXd get_laplace_mode(const std::string &fname,
                                 const stan::model::model_base &model) {
  std::stringstream msg;
  Eigen::VectorXd theta_hat;
  if (file::get_suffix(fname) == ".csv") {
    theta_hat = get_laplace_mode_csv(fname, model);
  } else if (file::get_suffix(fname) == ".json") {
    std::vector<double> unc_params
        = unconstrain_params_var_context(fname, model);
    theta_hat
        = Eigen::Map<Eigen::VectorXd>(unc_params.data(), unc_params.size());
  } else {
    msg << "Mode file must be CSV or JSON, found " << fname << std::endl;
    throw std::invalid_argument(msg.str());
  }
  return theta_hat;
}

/**
 * Extract and validate one or more sets of unconstrained parameters.
 * Input file contains single variable `params_r`, which is either
 * a vector or array of vectors of values for the model parameters
 * on the unconstrained scale.
 *
 * @param fname name of file which exists and has read perms
 * @param model Stan model
 * @return vector of vectors of parameter estimates
 */
std::vector<std::vector<double>> get_uparams_r(
    const std::string &fname, const stan::model::model_base &model) {
  size_t u_params_cols = 0;
  size_t u_params_rows = 0;
  std::vector<double> u_params_r;
  std::vector<size_t> dims_u_params_r;
  std::stringstream msg;

  std::shared_ptr<stan::io::var_context> upars_context = get_var_context(fname);
  u_params_r = upars_context->vals_r("params_r");
  if (u_params_r.size() == 0) {
    msg << "Unconstrained parameters file has no variable 'params_r' with "
           "unconstrained parameter values!";
    throw std::invalid_argument(msg.str());
  }
  // is input single vector of params or array of vectors?
  dims_u_params_r = upars_context->dims_r("params_r");
  u_params_rows = dims_u_params_r.size() == 2 ? dims_u_params_r[0] : 1;
  u_params_cols
      = dims_u_params_r.size() == 2 ? dims_u_params_r[1] : dims_u_params_r[0];
  size_t num_upars = model.num_params_r();
  if (u_params_cols != num_upars) {
    msg << "Incorrect number of unconstrained parameters provided! "
           "Model has "
        << num_upars << " parameters but " << u_params_cols << " were found.";
    throw std::invalid_argument(msg.str());
  }
  // brute force reshape
  std::vector<std::vector<double>> params_r_ind(
      u_params_rows, std::vector<double>(u_params_cols));
  size_t idx = 0;
  for (size_t i = 0; i < u_params_rows; ++i) {
    for (size_t j = 0; j < u_params_cols; ++j) {
      params_r_ind[i][j] = *(u_params_r.data() + idx);
      ++idx;
    }
  }
  return params_r_ind;
}

/**
 * Given a set of parameter values, call model's log_prob_grad
 * method and send output to a CSV file.
 *
 * @param model Stan model
 * @param jacobian jacobian adjustment flag
 * @param params_set array of unconstrained parameter values
 * @
 */
void services_log_prob_grad(const stan::model::model_base &model, bool jacobian,
                            std::vector<std::vector<double>> &params_set,
                            int sig_figs, std::ostream &output_stream) {
  // header row
  output_stream << std::setprecision(sig_figs) << "lp__,";
  std::vector<std::string> p_names;
  model.unconstrained_param_names(p_names, false, false);
  for (size_t i = 0; i < p_names.size(); ++i) {
    output_stream << "g_" << p_names[i];
    if (i == p_names.size() - 1)
      output_stream << "\n";
    else
      output_stream << ",";
  }
  // data row(s)
  std::vector<int> dummy_params_i;
  double lp;
  std::vector<double> gradients;
  for (auto &&params : params_set) {
    if (jacobian) {
      lp = stan::model::log_prob_grad<true, true>(model, params, dummy_params_i,
                                                  gradients);
    } else {
      lp = stan::model::log_prob_grad<true, false>(model, params,
                                                   dummy_params_i, gradients);
    }
    output_stream << lp << ",";
    std::copy(gradients.begin(), gradients.end() - 1,
              std::ostream_iterator<double>(output_stream, ","));
    output_stream << gradients.back() << "\n";
  }
}

/**
 * For sample and pathfinder methods, return number of
 * chains or pathfinders to run, otherwise return 1.
 * For the sampler, only adaptive NUTS-HMC with non-unit metric
 * supports multi-chain parallelization; if sampler config is invalid,
 * this will throw an error.
 *
 * @param parser user config
 * @param id id of initial chain or path. Only used for generated quantities
 * @return int num chains or paths
 */
inline unsigned int get_num_chains(argument_parser &parser, unsigned int id) {
  auto user_method = parser.arg("method");
  if (user_method->arg("pathfinder"))
    return get_arg_val<int_argument>(parser, "method", "pathfinder",
                                     "num_paths");

  auto gq_arg = user_method->arg("generate_quantities");
  if (gq_arg) {
    return static_cast<unsigned int>(
        get_arg_val<int_argument>(*gq_arg, "num_chains"));
  }
  auto sample_arg = user_method->arg("sample");
  if (!sample_arg) {
    return 1;
  }

  unsigned int num_chains
      = get_arg_val<int_argument>(*sample_arg, "num_chains");

  if (num_chains > 1) {
    list_argument *algo
        = dynamic_cast<list_argument *>(sample_arg->arg("algorithm"));
    if (algo->value() != "fixed_param"
        && get_arg_val<list_argument>(*algo, "hmc", "engine") == "static") {
      throw std::invalid_argument(
          "Argument 'num_chains' is unavailable for the 'static' HMC engine.");
    }
  }
  return num_chains;
}

/**
 * Check possible name conflicts between input and output files where both
 * are in Stan CSV format; if conflicts detected, quit with an error message.
 *
 * @param parser user config
 */
void check_file_config(argument_parser &parser) {
  std::string sample_file
      = get_arg_val<string_argument>(parser, "output", "file");
  file::validate_output_filename(sample_file);
  std::string diagnostic_file
      = get_arg_val<string_argument>(parser, "output", "diagnostic_file");
  file::validate_output_filename(diagnostic_file);
  auto user_method = parser.arg("method");

  std::string input_file_name = "";
  std::string input_file = "";
  if (user_method->arg("generate_quantities")) {
    input_file_name = "fitted_params";
    input_file = get_arg_val<string_argument>(
        parser, "method", "generate_quantities", "fitted_params");

  } else if (user_method->arg("laplace")) {
    input_file_name = "mode";
    input_file
        = get_arg_val<string_argument>(parser, "method", "laplace", "mode");
  } else if (user_method->arg("log_prob")) {
    input_file_name = "constrained_params";
    input_file = get_arg_val<string_argument>(parser, "method", "log_prob",
                                              "constrained_params");
  }

  if (!input_file_name.empty()) {
    if (file::check_approx_same_file(input_file, sample_file)) {
      std::stringstream msg;
      msg << "Filename conflict, " << input_file_name << " file " << input_file
          << " and output file names are identical, must be different."
          << std::endl;
      throw std::invalid_argument(msg.str());
    }
  }
}

template <typename T>
void init_null_writers(std::vector<T> &writers, size_t num_chains) {
  writers.reserve(num_chains);
  for (size_t i = 0; i < num_chains; ++i) {
    writers.emplace_back(nullptr);
  }
}

template <typename T, typename... Ts>
void init_filestream_writers(std::vector<T> &writers, unsigned int num_chains,
                             unsigned int id, std::string &filename,
                             std::string tag, std::string suffix, int sig_figs,
                             Ts &&... args) {
  writers.reserve(num_chains);
  auto filenames = file::make_filenames(filename, tag, suffix, num_chains, id);

  for (size_t i = 0; i < num_chains; ++i) {
    auto ofs = file::safe_create(filenames[i], sig_figs);
    writers.emplace_back(std::move(ofs), std::forward<Ts>(args)...);
  }
}

}  // namespace cmdstan

#endif
