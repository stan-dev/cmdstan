#ifndef CMDSTAN_HELPER_HPP
#define CMDSTAN_HELPER_HPP

#include <cmdstan/arguments/argument_parser.hpp>
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
    msg << "Model " << model.model_name() << " has no parameters, nothing to estimate." << std::endl;
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
 * Get params block from draws matrix
 */
void get_fitted_params(const std::string &fname,
                       const stan::model::model_base &model,
                       stan::io::stan_csv &fitted_params,
                       size_t &col_offset,
                       size_t &num_rows,
                       size_t &num_cols) {

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
      msg << "Mismatch between model and fitted_parameters csv file \""
          << fname << "\"" << std::endl;
      throw std::invalid_argument(msg.str());
    }
  }
}

/**
 * Get vector of parameter modes from StanCSV file
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
  boost::algorithm::split(names, line, boost::is_any_of(","), boost::token_compress_on);
  std::getline(in, line);
  std::vector<std::string> values;
  boost::algorithm::split(values, line, boost::is_any_of(","), boost::token_compress_on);
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
      msg << "Mismatch between model params and StanCSV file \""
          << fname << "\",  expecting param \"" << param_names[i]
          << "\", found \"" << names[i + col_offset] << "\"" << std::endl;
      throw std::invalid_argument(msg.str());
    }
    try {
      theta_hat[i] = std::stof(values[i + col_offset]);
    } catch (const std::exception &e) {
      msg << "Error parsing CSV file, bad value for " << param_names[i]  << std::endl;
      throw std::invalid_argument(msg.str());
    }
  }
}


/**
 * Get vector of parameter modes from JSON file
 */
void get_theta_hat_json(const std::string &fname,
                        const std::vector<std::string> &param_names,
                        Eigen::VectorXd &theta_hat) {

  std::ifstream stream = safe_open(fname);
  // parse json
  // iterate through param names
  // assign json vals to theta_hat
  stream.close();
}


Eigen::VectorXd get_laplace_mode(const std::string &fname,
                                 const stan::model::model_base &model) {
  std::stringstream msg;
  std::vector<std::string> param_names;
  get_constrained_params(model, param_names);
  Eigen::VectorXd theta_hat(param_names.size());
  std::string f2 = boost::to_lower_copy(fname);
  if (boost::ends_with(f2, ".csv"))
    get_theta_hat_csv(fname, param_names, theta_hat);
  else if (boost::ends_with(f2, ".json"))
    get_theta_hat_json(fname, param_names, theta_hat);
  else {
    msg << "Mode file must be CSV or JSON, found " << fname << std::endl;
    throw std::invalid_argument(msg.str());
  }
  return theta_hat;
}

#endif
