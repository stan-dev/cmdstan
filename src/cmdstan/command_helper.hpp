#ifndef CMDSTAN_HELPER_HPP
#define CMDSTAN_HELPER_HPP

#include <cmdstan/arguments/argument_parser.hpp>
#include <stan/io/dump.hpp>
#include <stan/io/ends_with.hpp>
#include <stan/io/json/json_data.hpp>
#include <stan/io/stan_csv_reader.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/model/model_base.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <rapidjson/document.h>

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
 * @tparam Args A paramter pack of const char*
 * @param arg_list The list argument to access the arg from
 * @param arg1 The name of the first argument to extract
 * @param args An optional pack of named arguments to access from the first arg.
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
 * @tparam Args A paramter pack of const char*
 * @param arg_list The list argument to access the arg from
 * @param arg1 The name of the first argument to extract
 * @param args An optional pack of named arguments to access from the first arg.
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
 * @tparam caster The type to cast the `argument` class in the list to.
 * @tparam Arg An object that inherits from `argument`.
 * @param argument holds the argument to access
 * @param arg_name The name of the argument to access.
 */
template <typename caster, typename Arg>
inline constexpr auto get_arg_val(Arg &&argument, const char *arg_name) {
  return dynamic_cast<std::decay_t<caster> *>(argument.arg(arg_name))->value();
}

/**
 * Given a list of arguments, index into the args and return the value held
 * by the underlying element in the list. Because all of the elements in
 * our list of command line arguments is an `argument` class with no
 * `value()` method, we must give the function the type of the argument class we
 * want to access.
 * @tparam caster The type to cast the `argument` class in the list to.
 * @tparam List A pointer or object that inherits from `argument`.
 * @param arg_list holds the arguments to access
 * @param args A parameter pack of names of arguments to index into.
 */
template <typename caster, typename List, typename... Args>
inline constexpr auto get_arg_val(List &&arg_list, Args &&... args) {
  return dynamic_cast<std::decay_t<caster> *>(get_arg(arg_list, args...))
      ->value();
}

using shared_context_ptr = std::shared_ptr<stan::io::var_context>;
/**
 * Given the name of a file, return a shared pointer holding the data contents.
 * @param file A system file to read from.
 */
inline shared_context_ptr get_var_context(const std::string file) {
  std::fstream stream(file.c_str(), std::fstream::in);
  if (file != "" && (stream.rdstate() & std::ifstream::failbit)) {
    std::stringstream msg;
    msg << "Can't open specified file, \"" << file << "\"" << std::endl;
    throw std::invalid_argument(msg.str());
  }
  if (stan::io::ends_with(".json", file)) {
    stan::json::json_data var_context(stream);
    return std::make_shared<stan::json::json_data>(var_context);
  }
  stan::io::dump var_context(stream);
  return std::make_shared<stan::io::dump>(var_context);
}

using context_vector = std::vector<shared_context_ptr>;
/**
 * Make a vector of shared pointers to contexts.
 * @param file The name of the file. For multi-chain we will attempt to find
 *  {file_name}_1{file_ending} and if that fails try to use the named file as
 *  the data for each chain.
 * @param num_chains The number of chains to run.
 * @return An std vector of shared pointers to var contexts
 */
context_vector get_vec_var_context(const std::string &file, size_t num_chains) {
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
  if (file == "") {
    using stan::io::dump;
    std::fstream stream(file.c_str(), std::fstream::in);
    return context_vector(num_chains, std::make_shared<dump>(dump(stream)));
  } else {
    size_t file_marker_pos = file.find_last_of(".");
    if (file_marker_pos > file.size()) {
      std::stringstream msg;
      msg << "Found: \"" << file
          << "\" but user specied files must end in .json or .R";
      throw std::invalid_argument(msg.str());
    }
    std::string file_name = file.substr(0, file_marker_pos);
    std::string file_ending = file.substr(file_marker_pos, file.size());
    if (file_ending != ".json" && file_ending != ".R") {
      std::stringstream msg;
      msg << "file ending of " << file_ending << " is not supported by cmdstan";
      throw std::invalid_argument(msg.str());
    }
    std::string file_1
        = std::string(file_name + "_" + std::to_string(1) + file_ending);
    std::fstream stream_1(file_1.c_str(), std::fstream::in);
    // Check if file_1 exists, if so then we'll assume num_chains of these
    // exist.
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
        std::string file_i
            = std::string(file_name + "_" + std::to_string(i) + file_ending);
        std::fstream stream_i(file_1.c_str(), std::fstream::in);
        // If any stream fails at this point something went wrong with file
        // names.
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
  using stan::io::dump;
  std::fstream stream(file.c_str(), std::fstream::in);
  return context_vector(num_chains, std::make_shared<dump>(dump(stream)));
}

/**
 * Get vector of constrainted parameter names from model.
 * Throws exception when model has no parameters.
 *
 * @param model the instantiated model
 * @param vector of constrained param names
 */
void get_constrained_params(const stan::model::model_base &model,
                            std::vector<std::string> &param_names) {
  model.constrained_param_names(param_names, false, false);
  if (param_names.size() < 1) {
    std::stringstream msg;
    msg << "Model " << model.model_name()
        << " has no parameters, nothing to estimate." << std::endl;
    throw std::invalid_argument(msg.str());
  }
}

/**
 * Opens input stream for file.
 * Throws exception if stream cannot be opened.
 *
 * @param fname name of file which exists and has read perms.
 * @return input stream
 */
std::ifstream safe_open(const std::string fname) {
  std::ifstream stream(fname.c_str());
  if (fname != "" && (stream.rdstate() & std::ifstream::failbit)) {
    std::stringstream msg;
    msg << "Can't open specified file, \"" << fname << "\"" << std::endl;
    throw std::invalid_argument(msg.str());
  }
  return stream;
}

/**
 * Parse a StanCSV output file created by the NUTS-HMC sampler and
 * identify the rows and columns in the data table which contain the
 * fitted estimated for the model parameters.
 * Throws an exception if the StanCSV parser cannot process the file.
 *
 * @param fname name of file which exists and has read perms.
 * @param model instantiated model
 * @param fitted_params struct which contains CSV header and data rows
 * @param col_offset first column of model outputs in the data table
 * @param num_rows total data table rows
 * @param num_cols total data table columns with parameter variable values
 */
void get_fitted_params(const std::string &fname,
                       const stan::model::model_base &model,
                       stan::io::stan_csv &fitted_params, size_t &col_offset,
                       size_t &num_rows, size_t &num_cols) {
  std::stringstream msg;
  std::vector<std::string> param_names;
  get_constrained_params(model, param_names);
  std::ifstream stream = safe_open(fname);

  // get matrix of draws from sample
  stan::io::stan_csv_reader::read_metadata(stream, fitted_params.metadata,
                                           &msg);
  if (!stan::io::stan_csv_reader::read_header(stream, fitted_params.header,
                                              &msg, false)) {
    msg << "Error reading fitted param names from sample csv file \"" << fname
        << "\"" << std::endl;
    throw std::invalid_argument(msg.str());
  }
  stan::io::stan_csv_reader::read_adaptation(stream, fitted_params.adaptation,
                                             &msg);
  fitted_params.timing.warmup = 0;
  fitted_params.timing.sampling = 0;
  stan::io::stan_csv_reader::read_samples(stream, fitted_params.samples,
                                          fitted_params.timing, &msg);
  stream.close();

  // compute offset, size of parameters block
  col_offset = 0;
  for (auto col_name : fitted_params.header) {
    if (boost::ends_with(col_name, "__")) {
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
 * Parse a StanCSV output file created by the optimizer and return a vector
 * containing the estimates of the model parameters.
 * Throws an exception if it cannot parse the CSV file.
 *
 * @param fname name of file which exists and has read perms.
 * @param param_names vector of model constrained parameter names.
 * @param theta_hat Eigen vector for estimated parameters
 */
void get_theta_hat_csv(const std::string &fname,
                       const std::vector<std::string> &param_names,
                       Eigen::VectorXd &theta_hat) {
  std::stringstream msg;
  std::ifstream in = safe_open(fname);
  std::string line;

  // skip initial comments
  while (in.peek() == '#')
    std::getline(in, line);

  // CSV header and optimization estimates
  std::getline(in, line);
  std::vector<std::string> names;
  boost::algorithm::split(names, line, boost::is_any_of(","),
                          boost::token_compress_on);
  std::getline(in, line);
  std::vector<std::string> values;
  boost::algorithm::split(values, line, boost::is_any_of(","),
                          boost::token_compress_on);
  in.close();

  size_t col_offset = 0;
  for (auto name : names) {
    if (boost::algorithm::ends_with(name, "__")) {
      col_offset++;
    } else {
      break;
    }
  }
  for (size_t i = 0; i < param_names.size(); ++i) {
    if (param_names[i].compare(names[i + col_offset]) != 0) {
      msg << "Mismatch between model params and StanCSV file \"" << fname
          << "\",  expecting param \"" << param_names[i] << "\", found \""
          << names[i + col_offset] << "\"" << std::endl;
      throw std::invalid_argument(msg.str());
    }
    try {
      theta_hat[i] = std::stod(values[i + col_offset]);
    } catch (const std::exception &e) {
      msg << "Error parsing CSV file, bad value for " << param_names[i]
          << std::endl;
      throw std::invalid_argument(msg.str());
    }
  }
}

/**
 * Parse a JSON file and extract the model parameter estimates.
 * Helper function get_var_context throws exception if cannot
 * open or parse JSON file.
 *
 * @param fname name of file which exists and has read perms.
 * @param param_names vector of model constrained parameter names.
 * @param theta_hat Eigen vector for estimated parameters
 */
void get_theta_hat_json(const std::string &fname,
                        const std::vector<std::string> &param_names,
                        Eigen::VectorXd &theta_hat) {
  std::vector<std::string> var_names;
  std::vector<std::string> splits;
  std::string cur_name = "";
  for (auto &&param_name : param_names) {
    boost::algorithm::split(splits, param_name, boost::is_any_of("."),
                            boost::token_compress_on);
    if (cur_name.compare(splits[0]) != 0)
      var_names.push_back(splits[0]);
    cur_name = splits[0];
  }

  std::shared_ptr<stan::io::var_context> context = get_var_context(fname);
  Eigen::Index offset = 0;
  for (auto &&var_name : var_names) {
    const auto param_vec = context->vals_r(var_name);
    for (Eigen::Index i = 0; i < param_vec.size(); ++i) {
      theta_hat[offset] = param_vec[i];
      ++offset;
    }
  }
}


/**
 * Parse contents of file containing estimate of parameter modes.
 *
 * @param fname name of file which exists and has read perms.
 * @param model Stan model
 * @return Eigen vector of parameter estimates.
 */
Eigen::VectorXd get_laplace_mode(const std::string &fname,
                                 const stan::model::model_base &model) {
  std::stringstream msg;
  std::vector<std::string> param_names;
  get_constrained_params(model, param_names);
  Eigen::VectorXd theta_hat(param_names.size());
  std::string f2 = boost::to_lower_copy(fname);
  if (boost::ends_with(f2, ".csv")) {
    get_theta_hat_csv(fname, param_names, theta_hat);
  } else if (boost::ends_with(f2, ".json")) {
    get_theta_hat_json(fname, param_names, theta_hat);
  } else {
    msg << "Mode file must be CSV or JSON, found " << fname << std::endl;
    throw std::invalid_argument(msg.str());
  }
  return theta_hat;
}

#endif
