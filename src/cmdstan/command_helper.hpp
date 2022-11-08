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
 * Get params block from draws matrix
 */
void get_fitted_params(const std::string &fname,
                       const stan::model::model_base &model,
                       stan::io::stan_csv &fitted_params,
                       size_t &col_offset,
                       size_t &num_rows,
                       size_t &num_cols) {
  // read sample from cmdstan csv output file
  std::stringstream msg;
  std::ifstream stream(fname.c_str());
  if (fname != "" && (stream.rdstate() & std::ifstream::failbit)) {
    msg << "Can't open specified file, \"" << fname << "\"" << std::endl;
    throw std::invalid_argument(msg.str());
  }
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
  // validate sample contents
  std::vector<std::string> param_names;
  model.constrained_param_names(param_names, false, false);
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
  // check that all parameter names are in sample, in order
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

Eigen::VectorXd get_laplace_mode(const std::string &fname,
                                 const stan::model::model_base &model) {
  // theta_hat is length num_constrained_params
  // instantiate theta_hat
  // call file-format appropriate parser
  Eigen::VectorXd foo(1);
  return foo;
}

/**
 * Get vector of parameter modes from StanCSV file
 */
void get_laplace_mode_csv(const std::string &fname,
                          const std::vector<std::string> &params,
                          Eigen::VectorXd &theta_hat) {

  // read up to header
  // get header, check names
  // get next line, split; get theta-hats
}

/**
 * Get vector of parameter modes from JSON file
 */
void get_laplace_mode_json(const std::string &fname,
                          const std::vector<std::string> &params,
                          Eigen::VectorXd &theta_hat) {

  // parse json
  // iterate through param names
  // assign json vals to theta_hat

}


#endif
