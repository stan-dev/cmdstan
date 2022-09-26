#ifndef CMDSTAN_COMMAND_HPP
#define CMDSTAN_COMMAND_HPP

#include <cmdstan/arguments/arg_data.hpp>
#include <cmdstan/arguments/arg_id.hpp>
#include <cmdstan/arguments/arg_init.hpp>
#include <cmdstan/arguments/arg_output.hpp>
#include <cmdstan/arguments/arg_num_threads.hpp>
#include <cmdstan/arguments/arg_num_chains.hpp>
#include <cmdstan/arguments/arg_random.hpp>
#include <cmdstan/arguments/arg_opencl.hpp>
#include <cmdstan/arguments/arg_profile_file.hpp>
#include <cmdstan/arguments/argument_parser.hpp>
#include <cmdstan/write_chain.hpp>
#include <cmdstan/write_datetime.hpp>
#include <cmdstan/write_model_compile_info.hpp>
#include <cmdstan/write_model.hpp>
#include <cmdstan/write_opencl_device.hpp>
#include <cmdstan/write_parallel_info.hpp>
#include <cmdstan/write_profiling.hpp>
#include <cmdstan/write_stan.hpp>
#include <cmdstan/write_stan_flags.hpp>
#include <stan/callbacks/interrupt.hpp>
#include <stan/callbacks/logger.hpp>
#include <stan/callbacks/stream_logger.hpp>
#include <stan/callbacks/unique_stream_writer.hpp>
#include <stan/callbacks/stream_writer.hpp>
#include <stan/callbacks/writer.hpp>
#include <stan/io/dump.hpp>
#include <stan/io/ends_with.hpp>
#include <stan/io/stan_csv_reader.hpp>
#include <stan/io/json/json_data.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/model/model_base.hpp>
#include <stan/services/diagnose/diagnose.hpp>
#include <stan/services/experimental/advi/fullrank.hpp>
#include <stan/services/experimental/advi/meanfield.hpp>
#include <stan/services/optimize/bfgs.hpp>
#include <stan/services/optimize/lbfgs.hpp>
#include <stan/services/optimize/newton.hpp>
#include <stan/services/sample/fixed_param.hpp>
#include <stan/services/sample/hmc_nuts_dense_e.hpp>
#include <stan/services/sample/hmc_nuts_dense_e_adapt.hpp>
#include <stan/services/sample/hmc_nuts_diag_e.hpp>
#include <stan/services/sample/hmc_nuts_diag_e_adapt.hpp>
#include <stan/services/sample/hmc_nuts_unit_e.hpp>
#include <stan/services/sample/hmc_nuts_unit_e_adapt.hpp>
#include <stan/services/sample/hmc_static_dense_e.hpp>
#include <stan/services/sample/hmc_static_dense_e_adapt.hpp>
#include <stan/services/sample/hmc_static_diag_e.hpp>
#include <stan/services/sample/hmc_static_diag_e_adapt.hpp>
#include <stan/services/sample/hmc_static_unit_e.hpp>
#include <stan/services/sample/hmc_static_unit_e_adapt.hpp>
#include <stan/services/sample/standalone_gqs.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <stan/math/prim/core/init_threadpool_tbb.hpp>

#ifdef STAN_MPI
#include <stan/math/prim/functor/mpi_cluster.hpp>
#include <stan/math/prim/functor/mpi_command.hpp>
#include <stan/math/prim/functor/mpi_distributed_apply.hpp>
#endif

// forward declaration for function defined in another translation unit
stan::model::model_base &new_model(stan::io::var_context &data_context,
                                   unsigned int seed, std::ostream *msg_stream);
stan::math::profile_map &get_stan_profile_data();

namespace cmdstan {

struct return_codes {
  enum { OK = 0, NOT_OK = 1 };
};

#ifdef STAN_MPI
stan::math::mpi_cluster &get_mpi_cluster() {
  static stan::math::mpi_cluster cluster;
  return cluster;
}
#endif

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

int command(int argc, const char *argv[]) {
  stan::callbacks::stream_writer info(std::cout);
  stan::callbacks::stream_writer err(std::cerr);
  stan::callbacks::stream_logger logger(std::cout, std::cout, std::cout,
                                        std::cerr, std::cerr);

#ifdef STAN_MPI
  stan::math::mpi_cluster &cluster = get_mpi_cluster();
  cluster.listen();
  if (cluster.rank_ != 0)
    return 0;
#endif

  // Read arguments
  std::vector<argument *> valid_arguments;
  valid_arguments.push_back(new arg_id());
  valid_arguments.push_back(new arg_data());
  valid_arguments.push_back(new arg_init());
  valid_arguments.push_back(new arg_random());
  valid_arguments.push_back(new arg_output());
  valid_arguments.push_back(new arg_num_threads());
#ifdef STAN_OPENCL
  valid_arguments.push_back(new arg_opencl());
#endif
  argument_parser parser(valid_arguments);
  int err_code = parser.parse_args(argc, argv, info, err);
  if (err_code == stan::services::error_codes::USAGE) {
    if (argc > 1)
      std::cerr << "Failed to parse command arguments, cannot run model."
                << std::endl;
    return return_codes::NOT_OK;
  } else if (err_code != 0) {
    std::cerr << "Unexpected failure, quitting." << std::endl;
    return return_codes::NOT_OK;
  }
  if (parser.help_printed())
    return return_codes::OK;

  int num_threads = get_arg_val<int_argument>(parser, "num_threads");
  // Need to make sure these two ways to set thread # match.
  int env_threads = stan::math::internal::get_num_threads();
  if (env_threads != num_threads) {
    if (env_threads != 1) {
      std::stringstream thread_msg;
      thread_msg << "STAN_NUM_THREADS= " << env_threads
                 << " but argument num_threads= " << num_threads
                 << ". Please either only set one or make sure they are equal.";
      throw std::invalid_argument(thread_msg.str());
    }
  }
  stan::math::init_threadpool_tbb(num_threads);

  unsigned int num_chains = 1;
  auto user_method = parser.arg("method");
  // num_chains > 1 is only supported in diag_e and dense_e of hmc
  if (user_method->arg("sample")) {
    num_chains
        = get_arg_val<int_argument>(parser, "method", "sample", "num_chains");
    auto sample_arg = parser.arg("method")->arg("sample");
    list_argument *algo
        = dynamic_cast<list_argument *>(sample_arg->arg("algorithm"));
    categorical_argument *adapt
        = dynamic_cast<categorical_argument *>(sample_arg->arg("adapt"));
    const bool adapt_engaged
        = dynamic_cast<bool_argument *>(adapt->arg("engaged"))->value();
    const bool is_hmc = algo->value() == "hmc";
    if (num_chains > 1) {
      if (is_hmc && adapt_engaged) {
        list_argument *engine
            = dynamic_cast<list_argument *>(algo->arg("hmc")->arg("engine"));
        list_argument *metric
            = dynamic_cast<list_argument *>(algo->arg("hmc")->arg("metric"));
        if (engine->value() != "nuts"
            && (metric->value() != "dense_e" || metric->value() == "diag_e")) {
          throw std::invalid_argument(
              "Argument 'num_chains' can currently only be used for NUTS with "
              "adaptation and dense_e or diag_e metric");
        }
      } else {
        throw std::invalid_argument(
            "Argument 'num_chains' can currently only be used for HMC with "
            "adaptation engaged");
      }
    }
  }
  arg_seed *random_arg
      = dynamic_cast<arg_seed *>(parser.arg("random")->arg("seed"));
  unsigned int random_seed = random_arg->random_value();

#ifdef STAN_OPENCL
  int_argument *opencl_device_id
      = dynamic_cast<int_argument *>(parser.arg("opencl")->arg("device"));
  int_argument *opencl_platform_id
      = dynamic_cast<int_argument *>(parser.arg("opencl")->arg("platform"));

  // Either both device and platform are set or neither in which case we default
  // to compile-time constants
  if ((opencl_device_id->is_default() && !opencl_platform_id->is_default())
      || (!opencl_device_id->is_default()
          && opencl_platform_id->is_default())) {
    std::cerr << "Please set both device and platform OpenCL IDs." << std::endl;
    return return_codes::NOT_OK;
  } else if (!opencl_device_id->is_default()
             && !opencl_platform_id->is_default()) {
    stan::math::opencl_context.select_device(opencl_platform_id->value(),
                                             opencl_device_id->value());
  }
#endif

  parser.print(info);
  write_parallel_info(info);
  write_opencl_device(info);
  info();

  std::stringstream msg;

  // Cross-check arguments
  if (parser.arg("method")->arg("generate_quantities")) {
    std::string fitted_sample_fname
        = dynamic_cast<string_argument *>(parser.arg("method")
                                              ->arg("generate_quantities")
                                              ->arg("fitted_params"))
              ->value();
    std::string output_fname
        = dynamic_cast<string_argument *>(parser.arg("output")->arg("file"))
              ->value();
    if (fitted_sample_fname.compare(output_fname) == 0) {
      msg << "Filename conflict, fitted_params file " << output_fname
          << " and output file have same name, must be different." << std::endl;
      throw std::invalid_argument(msg.str());
    }
  }

  // Open output streams
  stan::callbacks::writer init_writer;
  stan::callbacks::interrupt interrupt;

  //////////////////////////////////////////////////
  //                Initialize Model              //
  //////////////////////////////////////////////////
  std::string filename = get_arg_val<string_argument>(parser, "data", "file");

  std::shared_ptr<stan::io::var_context> var_context
      = get_var_context(filename);

  // Instantiate model
  stan::model::model_base &model
      = new_model(*var_context, random_seed, &std::cout);

  std::vector<std::string> model_compile_info = model.model_compile_info();

  //////////////////////////////////////////////////
  //                Initialize Writers            //
  //////////////////////////////////////////////////

  std::string output_file
      = get_arg_val<string_argument>(parser, "output", "file");
  if (output_file == "") {
    throw std::invalid_argument(
        std::string("File output name must not be blank"));
  }
  std::string output_name;
  std::string output_ending;
  size_t output_marker_pos = output_file.find_last_of(".");
  if (output_marker_pos > output_file.size()) {
    output_name = output_file;
    output_ending = "";
  } else {
    output_name = output_file.substr(0, output_marker_pos);
    output_ending = output_file.substr(output_marker_pos, output_file.size());
  }

  std::string diagnostic_file
      = get_arg_val<string_argument>(parser, "output", "diagnostic_file");
  size_t diagnostic_marker_pos = diagnostic_file.find_last_of(".");
  std::string diagnostic_name;
  std::string diagnostic_ending;
  // no . seperator found.
  if (diagnostic_marker_pos > diagnostic_file.size()) {
    diagnostic_name = diagnostic_file;
    diagnostic_ending = "";
  } else {
    diagnostic_name = diagnostic_file.substr(0, diagnostic_marker_pos);
    diagnostic_ending
        = diagnostic_file.substr(diagnostic_marker_pos, diagnostic_file.size());
  }

  std::vector<stan::callbacks::unique_stream_writer<std::ostream>>
      sample_writers;
  sample_writers.reserve(num_chains);
  std::vector<stan::callbacks::unique_stream_writer<std::ostream>>
      diagnostic_writers;
  diagnostic_writers.reserve(num_chains);
  std::vector<stan::callbacks::writer> init_writers{num_chains,
                                                    stan::callbacks::writer{}};
  unsigned int id = dynamic_cast<int_argument *>(parser.arg("id"))->value();
  int_argument *sig_figs_arg
      = dynamic_cast<int_argument *>(parser.arg("output")->arg("sig_figs"));
  auto name_iterator = [num_chains, id](auto i) {
    if (num_chains == 1) {
      return std::string("");
    } else {
      return std::string("_" + std::to_string(i + id));
    }
  };
  for (int i = 0; i < num_chains; i++) {
    auto output_filename = output_name + name_iterator(i) + output_ending;
    auto unique_fstream
        = std::make_unique<std::fstream>(output_filename, std::fstream::out);
    if (!sig_figs_arg->is_default()) {
      (*unique_fstream.get()) << std::setprecision(sig_figs_arg->value());
    }
    sample_writers.emplace_back(std::move(unique_fstream), "# ");
    if (diagnostic_file != "") {
      auto diagnostic_filename
          = diagnostic_name + name_iterator(i) + diagnostic_ending;
      diagnostic_writers.emplace_back(
          std::make_unique<std::fstream>(diagnostic_filename,
                                         std::fstream::out),
          "# ");
    } else {
      diagnostic_writers.emplace_back(nullptr, "# ");
    }
  }
  for (int i = 0; i < num_chains; i++) {
    write_stan(sample_writers[i]);
    write_model(sample_writers[i], model.model_name());
    write_datetime(sample_writers[i]);
    parser.print(sample_writers[i]);
    write_parallel_info(sample_writers[i]);
    write_opencl_device(sample_writers[i]);
    write_compile_info(sample_writers[i], model_compile_info);
    write_stan(diagnostic_writers[i]);
    write_model(diagnostic_writers[i], model.model_name());
    parser.print(diagnostic_writers[i]);
  }

  int refresh
      = dynamic_cast<int_argument *>(parser.arg("output")->arg("refresh"))
            ->value();

  // Read initial parameter values or user-specified radius
  std::string init
      = dynamic_cast<string_argument *>(parser.arg("init"))->value();
  double init_radius = 2.0;
  // argument "init" can be non-negative number of filename
  try {
    init_radius = boost::lexical_cast<double>(init);
    init = "";
  } catch (const boost::bad_lexical_cast &e) {
  }
  std::vector<std::shared_ptr<stan::io::var_context>> init_contexts
      = get_vec_var_context(init, num_chains);
  int return_code = stan::services::error_codes::CONFIG;
  if (user_method->arg("generate_quantities")) {
    // read sample from cmdstan csv output file
    string_argument *fitted_params_file = dynamic_cast<string_argument *>(
        parser.arg("method")->arg("generate_quantities")->arg("fitted_params"));
    if (fitted_params_file->is_default()) {
      msg << "Missing fitted_params argument, cannot run generate_quantities "
             "without fitted sample.";
      throw std::invalid_argument(msg.str());
    }
    // read sample from cmdstan csv output file
    std::string fname(fitted_params_file->value());
    std::ifstream stream(fname.c_str());
    if (fname != "" && (stream.rdstate() & std::ifstream::failbit)) {
      msg << "Can't open specified file, \"" << fname << "\"" << std::endl;
      throw std::invalid_argument(msg.str());
    }
    stan::io::stan_csv fitted_params;
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
    std::vector<std::string> param_names;
    model.constrained_param_names(param_names, false, false);
    size_t meta_cols = 0;
    for (auto col_name : fitted_params.header) {
      if (boost::algorithm::ends_with(col_name, "__")) {
        meta_cols++;
      } else {
        break;
      }
    }
    size_t num_cols = param_names.size();
    size_t num_rows = fitted_params.samples.rows();
    // check that all parameter names are in sample, in order
    if (num_cols + meta_cols > fitted_params.header.size()) {
      msg << "Mismatch between model and fitted_parameters csv file \"" << fname
          << "\"" << std::endl;
      throw std::invalid_argument(msg.str());
    }
    for (size_t i = 0; i < num_cols; ++i) {
      if (param_names[i].compare(fitted_params.header[i + meta_cols]) != 0) {
        msg << "Mismatch between model and fitted_parameters csv file \""
            << fname << "\"" << std::endl;
        throw std::invalid_argument(msg.str());
      }
    }
    return_code = stan::services::standalone_generate(
        model, fitted_params.samples.block(0, meta_cols, num_rows, num_cols),
        random_seed, interrupt, logger, sample_writers[0]);
  } else if (user_method->arg("log_prob")) {
    string_argument *upars_file = dynamic_cast<string_argument *>(
        parser.arg("method")->arg("log_prob")->arg("unconstrained_params"));
    string_argument *cpars_file = dynamic_cast<string_argument *>(
        parser.arg("method")->arg("log_prob")->arg("constrained_params"));
    bool jacobian_adjust
        = dynamic_cast<bool_argument *>(
              parser.arg("method")->arg("log_prob")->arg("jacobian_adjust"))
              ->value();
    if (upars_file->is_default() && cpars_file->is_default()) {
      msg << "No input parameters provided, cannot calculate log probability "
             "density";
      throw std::invalid_argument(msg.str());
    }

    size_t u_params_vec_size = 0;
    size_t u_params_size = 0;
    std::vector<double> u_params_r;
    std::vector<size_t> dims_u_params_r;
    if (!(upars_file->is_default())) {
      std::string u_fname(upars_file->value());
      std::ifstream u_stream(u_fname.c_str());

      std::shared_ptr<stan::io::var_context> upars_context
          = get_var_context(u_fname);

      u_params_r = (*upars_context).vals_r("params_r");
      dims_u_params_r = (*upars_context).dims_r("params_r");

      // Detect whether multiple sets of parameter values have been passed
      // and set the sizes accordingly
      u_params_vec_size = dims_u_params_r.size() == 2 ? dims_u_params_r[0] : 1;
      u_params_size = dims_u_params_r.size() == 2 ? dims_u_params_r[1]
                                                  : dims_u_params_r[0];
    }

    // Store names and dims for constructing array_var_context
    // and unconstraining transform
    std::vector<std::string> param_names;
    std::vector<std::vector<size_t>> param_dimss;
    stan::services::get_model_parameters(model, param_names, param_dimss);

    if (u_params_size > 0 && u_params_size != param_names.size()) {
      msg << "Incorrect number of unconstrained parameters provided! "
             "Model has "
          << param_names.size() << " parameters but " << u_params_size
          << " were found.";
      throw std::invalid_argument(msg.str());
    }

    size_t c_params_vec_size = 0;
    size_t c_params_size = 0;
    std::vector<double> c_params_r;
    std::vector<size_t> dims_c_params_r;
    if (!(cpars_file->is_default())) {
      std::string c_fname(cpars_file->value());
      std::ifstream c_stream(c_fname.c_str());

      std::shared_ptr<stan::io::var_context> cpars_context
          = get_var_context(c_fname);

      c_params_r = (*cpars_context).vals_r("params_r");
      dims_c_params_r = (*cpars_context).dims_r("params_r");

      c_params_vec_size = dims_c_params_r.size() == 2 ? dims_c_params_r[0] : 1;
      c_params_size = dims_c_params_r.size() == 2 ? dims_c_params_r[1]
                                                  : dims_c_params_r[0];
    }

    if (c_params_size > 0 && c_params_size != param_names.size()) {
      msg << "Incorrect number of constrained parameters provided! "
             "Model has "
          << param_names.size() << " parameters but " << c_params_size
          << " were found.";
      throw std::invalid_argument(msg.str());
    }

    // Store in single nested array to allow single loop for calc and print
    size_t num_par_sets = c_params_vec_size + u_params_vec_size;
    std::vector<std::vector<double>> params_r_ind(num_par_sets);

    // Use Map with inner stride to operate on all values from parameter set
    using StrideT = Eigen::Stride<1, Eigen::Dynamic>;
    std::vector<int> dummy_params_i;
    for (size_t i = 0; i < c_params_vec_size; i++) {
      Eigen::Map<Eigen::VectorXd, 0, StrideT> map_r(
          c_params_r.data() + i, c_params_size, StrideT(1, c_params_vec_size));

      stan::io::array_var_context context(param_names, map_r, param_dimss);
      model.transform_inits(context, dummy_params_i, params_r_ind[i], &msg);
    }

    for (size_t i = c_params_vec_size; i < num_par_sets; i++) {
      size_t iter = i - c_params_vec_size;
      Eigen::Map<Eigen::VectorXd, 0, StrideT> map_r(
          u_params_r.data() + iter, u_params_size,
          StrideT(1, u_params_vec_size));

      params_r_ind[i] = stan::math::to_array_1d(map_r);
    }

    std::string grad_output_file = get_arg_val<string_argument>(
        parser, "output", "log_prob_output_file");
    std::ofstream output_stream(grad_output_file);
    output_stream << std::setprecision(sig_figs_arg->value()) << "lp_,";

    std::vector<std::string> p_names;
    model.constrained_param_names(p_names, false, false);
    for (size_t i = 1; i < p_names.size(); i++) {
      output_stream << "g_" << p_names[i] << ",";
    }
    output_stream << "g_" << p_names.back() << "\n";
    try {
      double lp;
      std::vector<double> gradients;
      for (size_t i = 0; i < num_par_sets; i++) {
        if (jacobian_adjust) {
          lp = stan::model::log_prob_grad<false, true>(
              model, params_r_ind[i], dummy_params_i, gradients);
        } else {
          lp = stan::model::log_prob_grad<false, false>(
              model, params_r_ind[i], dummy_params_i, gradients);
        }

        output_stream << lp << ",";

        std::copy(gradients.begin(), gradients.end() - 1,
                  std::ostream_iterator<double>(output_stream, ","));
        output_stream << gradients.back();
        output_stream << "\n";
      }
      output_stream.close();
      return stan::services::error_codes::error_codes::OK;
    } catch (const std::exception &e) {
      output_stream.close();
      return stan::services::error_codes::error_codes::DATAERR;
    }
  } else if (user_method->arg("diagnose")) {
    list_argument *test = dynamic_cast<list_argument *>(
        parser.arg("method")->arg("diagnose")->arg("test"));

    if (test->value() == "gradient") {
      double epsilon
          = dynamic_cast<real_argument *>(test->arg("gradient")->arg("epsilon"))
                ->value();
      double error
          = dynamic_cast<real_argument *>(test->arg("gradient")->arg("error"))
                ->value();
      return_code = stan::services::diagnose::diagnose(
          model, *(init_contexts[0]), random_seed, id, init_radius, epsilon,
          error, interrupt, logger, init_writers[0], sample_writers[0]);
    }
  } else if (user_method->arg("optimize")) {
    list_argument *algo = dynamic_cast<list_argument *>(
        parser.arg("method")->arg("optimize")->arg("algorithm"));
    int num_iterations = dynamic_cast<int_argument *>(
                             parser.arg("method")->arg("optimize")->arg("iter"))
                             ->value();
    bool save_iterations
        = dynamic_cast<bool_argument *>(
              parser.arg("method")->arg("optimize")->arg("save_iterations"))
              ->value();

    if (algo->value() == "newton") {
      return_code = stan::services::optimize::newton(
          model, *(init_contexts[0]), random_seed, id, init_radius,
          num_iterations, save_iterations, interrupt, logger, init_writers[0],
          sample_writers[0]);
    } else if (algo->value() == "bfgs") {
      double init_alpha
          = dynamic_cast<real_argument *>(algo->arg("bfgs")->arg("init_alpha"))
                ->value();
      double tol_obj
          = dynamic_cast<real_argument *>(algo->arg("bfgs")->arg("tol_obj"))
                ->value();
      double tol_rel_obj
          = dynamic_cast<real_argument *>(algo->arg("bfgs")->arg("tol_rel_obj"))
                ->value();
      double tol_grad
          = dynamic_cast<real_argument *>(algo->arg("bfgs")->arg("tol_grad"))
                ->value();
      double tol_rel_grad = dynamic_cast<real_argument *>(
                                algo->arg("bfgs")->arg("tol_rel_grad"))
                                ->value();
      double tol_param
          = dynamic_cast<real_argument *>(algo->arg("bfgs")->arg("tol_param"))
                ->value();

      return_code = stan::services::optimize::bfgs(
          model, *(init_contexts[0]), random_seed, id, init_radius, init_alpha,
          tol_obj, tol_rel_obj, tol_grad, tol_rel_grad, tol_param,
          num_iterations, save_iterations, refresh, interrupt, logger,
          init_writers[0], sample_writers[0]);
    } else if (algo->value() == "lbfgs") {
      int history_size = dynamic_cast<int_argument *>(
                             algo->arg("lbfgs")->arg("history_size"))
                             ->value();
      double init_alpha
          = dynamic_cast<real_argument *>(algo->arg("lbfgs")->arg("init_alpha"))
                ->value();
      double tol_obj
          = dynamic_cast<real_argument *>(algo->arg("lbfgs")->arg("tol_obj"))
                ->value();
      double tol_rel_obj = dynamic_cast<real_argument *>(
                               algo->arg("lbfgs")->arg("tol_rel_obj"))
                               ->value();
      double tol_grad
          = dynamic_cast<real_argument *>(algo->arg("lbfgs")->arg("tol_grad"))
                ->value();
      double tol_rel_grad = dynamic_cast<real_argument *>(
                                algo->arg("lbfgs")->arg("tol_rel_grad"))
                                ->value();
      double tol_param
          = dynamic_cast<real_argument *>(algo->arg("lbfgs")->arg("tol_param"))
                ->value();

      return_code = stan::services::optimize::lbfgs(
          model, *(init_contexts[0]), random_seed, id, init_radius,
          history_size, init_alpha, tol_obj, tol_rel_obj, tol_grad,
          tol_rel_grad, tol_param, num_iterations, save_iterations, refresh,
          interrupt, logger, init_writers[0], sample_writers[0]);
    }
  } else if (user_method->arg("sample")) {
    auto sample_arg = parser.arg("method")->arg("sample");
    int num_warmup
        = dynamic_cast<int_argument *>(sample_arg->arg("num_warmup"))->value();
    int num_samples
        = dynamic_cast<int_argument *>(sample_arg->arg("num_samples"))->value();
    int num_thin
        = dynamic_cast<int_argument *>(sample_arg->arg("thin"))->value();
    bool save_warmup
        = dynamic_cast<bool_argument *>(sample_arg->arg("save_warmup"))
              ->value();
    list_argument *algo
        = dynamic_cast<list_argument *>(sample_arg->arg("algorithm"));
    categorical_argument *adapt
        = dynamic_cast<categorical_argument *>(sample_arg->arg("adapt"));
    bool adapt_engaged
        = dynamic_cast<bool_argument *>(adapt->arg("engaged"))->value();

    if (model.num_params_r() == 0 || algo->value() == "fixed_param") {
      if (algo->value() != "fixed_param") {
        info(
            "Model contains no parameters, running fixed_param sampler, "
            "no updates to Markov chain");
        if (num_chains > 1) {
          throw std::invalid_argument(
              "Argument 'num_chains' can currently only be used for HMC with "
              "adaptation engaged. This model has no parameters, which "
              "currently means only the fixed_param algorithm can be used");
        }
      }
      return_code = stan::services::sample::fixed_param(
          model, *(init_contexts[0]), random_seed, id, init_radius, num_samples,
          num_thin, refresh, interrupt, logger, init_writers[0],
          sample_writers[0], diagnostic_writers[0]);
    } else if (algo->value() == "hmc") {
      list_argument *engine
          = dynamic_cast<list_argument *>(algo->arg("hmc")->arg("engine"));

      list_argument *metric
          = dynamic_cast<list_argument *>(algo->arg("hmc")->arg("metric"));
      string_argument *metric_file = dynamic_cast<string_argument *>(
          algo->arg("hmc")->arg("metric_file"));
      bool metric_supplied = !metric_file->is_default();
      std::string metric_filename(
          dynamic_cast<string_argument *>(algo->arg("hmc")->arg("metric_file"))
              ->value());
      context_vector metric_contexts
          = get_vec_var_context(metric_filename, num_chains);
      categorical_argument *adapt
          = dynamic_cast<categorical_argument *>(sample_arg->arg("adapt"));
      categorical_argument *hmc
          = dynamic_cast<categorical_argument *>(algo->arg("hmc"));
      double stepsize
          = dynamic_cast<real_argument *>(hmc->arg("stepsize"))->value();
      double stepsize_jitter
          = dynamic_cast<real_argument *>(hmc->arg("stepsize_jitter"))->value();
      if (adapt_engaged == true && num_warmup == 0) {
        info(
            "The number of warmup samples (num_warmup) must be greater than "
            "zero if adaptation is enabled.");
        return_code = stan::services::error_codes::CONFIG;
      } else if (engine->value() == "nuts" && metric->value() == "dense_e"
                 && adapt_engaged == false && metric_supplied == false) {
        int max_depth = dynamic_cast<int_argument *>(
                            dynamic_cast<categorical_argument *>(
                                algo->arg("hmc")->arg("engine")->arg("nuts"))
                                ->arg("max_depth"))
                            ->value();
        return_code = stan::services::sample::hmc_nuts_dense_e(
            model, *(init_contexts[0]), random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, interrupt, logger, init_writers[0],
            sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "nuts" && metric->value() == "dense_e"
                 && adapt_engaged == false && metric_supplied == true) {
        int max_depth = dynamic_cast<int_argument *>(
                            dynamic_cast<categorical_argument *>(
                                algo->arg("hmc")->arg("engine")->arg("nuts"))
                                ->arg("max_depth"))
                            ->value();
        return_code = stan::services::sample::hmc_nuts_dense_e(
            model, *(init_contexts[0]), *(metric_contexts[0]), random_seed, id,
            init_radius, num_warmup, num_samples, num_thin, save_warmup,
            refresh, stepsize, stepsize_jitter, max_depth, interrupt, logger,
            init_writers[0], sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "nuts" && metric->value() == "dense_e"
                 && adapt_engaged == true && metric_supplied == false) {
        int max_depth = dynamic_cast<int_argument *>(
                            dynamic_cast<categorical_argument *>(
                                algo->arg("hmc")->arg("engine")->arg("nuts"))
                                ->arg("max_depth"))
                            ->value();
        double delta
            = dynamic_cast<real_argument *>(adapt->arg("delta"))->value();
        double gamma
            = dynamic_cast<real_argument *>(adapt->arg("gamma"))->value();
        double kappa
            = dynamic_cast<real_argument *>(adapt->arg("kappa"))->value();
        double t0 = dynamic_cast<real_argument *>(adapt->arg("t0"))->value();
        unsigned int init_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("init_buffer"))
                  ->value();
        unsigned int term_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("term_buffer"))
                  ->value();
        unsigned int window
            = dynamic_cast<u_int_argument *>(adapt->arg("window"))->value();
        return_code = stan::services::sample::hmc_nuts_dense_e_adapt(
            model, num_chains, init_contexts, random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, delta, gamma, kappa, t0, init_buffer,
            term_buffer, window, interrupt, logger, init_writers,
            sample_writers, diagnostic_writers);
      } else if (engine->value() == "nuts" && metric->value() == "dense_e"
                 && adapt_engaged == true && metric_supplied == true) {
        int max_depth = dynamic_cast<int_argument *>(
                            dynamic_cast<categorical_argument *>(
                                algo->arg("hmc")->arg("engine")->arg("nuts"))
                                ->arg("max_depth"))
                            ->value();
        double delta
            = dynamic_cast<real_argument *>(adapt->arg("delta"))->value();
        double gamma
            = dynamic_cast<real_argument *>(adapt->arg("gamma"))->value();
        double kappa
            = dynamic_cast<real_argument *>(adapt->arg("kappa"))->value();
        double t0 = dynamic_cast<real_argument *>(adapt->arg("t0"))->value();
        unsigned int init_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("init_buffer"))
                  ->value();
        unsigned int term_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("term_buffer"))
                  ->value();
        unsigned int window
            = dynamic_cast<u_int_argument *>(adapt->arg("window"))->value();
        return_code = stan::services::sample::hmc_nuts_dense_e_adapt(
            model, num_chains, init_contexts, metric_contexts, random_seed, id,
            init_radius, num_warmup, num_samples, num_thin, save_warmup,
            refresh, stepsize, stepsize_jitter, max_depth, delta, gamma, kappa,
            t0, init_buffer, term_buffer, window, interrupt, logger,
            init_writers, sample_writers, diagnostic_writers);
      } else if (engine->value() == "nuts" && metric->value() == "diag_e"
                 && adapt_engaged == false && metric_supplied == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("nuts"));
        int max_depth
            = dynamic_cast<int_argument *>(base->arg("max_depth"))->value();
        return_code = stan::services::sample::hmc_nuts_diag_e(
            model, *(init_contexts[0]), random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, interrupt, logger, init_writers[0],
            sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "nuts" && metric->value() == "diag_e"
                 && adapt_engaged == false && metric_supplied == true) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("nuts"));
        int max_depth
            = dynamic_cast<int_argument *>(base->arg("max_depth"))->value();
        return_code = stan::services::sample::hmc_nuts_diag_e(
            model, *(init_contexts[0]), *(metric_contexts[0]), random_seed, id,
            init_radius, num_warmup, num_samples, num_thin, save_warmup,
            refresh, stepsize, stepsize_jitter, max_depth, interrupt, logger,
            init_writers[0], sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "nuts" && metric->value() == "diag_e"
                 && adapt_engaged == true && metric_supplied == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("nuts"));
        int max_depth
            = dynamic_cast<int_argument *>(base->arg("max_depth"))->value();
        double delta
            = dynamic_cast<real_argument *>(adapt->arg("delta"))->value();
        double gamma
            = dynamic_cast<real_argument *>(adapt->arg("gamma"))->value();
        double kappa
            = dynamic_cast<real_argument *>(adapt->arg("kappa"))->value();
        double t0 = dynamic_cast<real_argument *>(adapt->arg("t0"))->value();
        unsigned int init_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("init_buffer"))
                  ->value();
        unsigned int term_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("term_buffer"))
                  ->value();
        unsigned int window
            = dynamic_cast<u_int_argument *>(adapt->arg("window"))->value();
        return_code = stan::services::sample::hmc_nuts_diag_e_adapt(
            model, num_chains, init_contexts, random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, delta, gamma, kappa, t0, init_buffer,
            term_buffer, window, interrupt, logger, init_writers,
            sample_writers, diagnostic_writers);
      } else if (engine->value() == "nuts" && metric->value() == "diag_e"
                 && adapt_engaged == true && metric_supplied == true) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("nuts"));
        int max_depth
            = dynamic_cast<int_argument *>(base->arg("max_depth"))->value();
        double delta
            = dynamic_cast<real_argument *>(adapt->arg("delta"))->value();
        double gamma
            = dynamic_cast<real_argument *>(adapt->arg("gamma"))->value();
        double kappa
            = dynamic_cast<real_argument *>(adapt->arg("kappa"))->value();
        double t0 = dynamic_cast<real_argument *>(adapt->arg("t0"))->value();
        unsigned int init_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("init_buffer"))
                  ->value();
        unsigned int term_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("term_buffer"))
                  ->value();
        unsigned int window
            = dynamic_cast<u_int_argument *>(adapt->arg("window"))->value();
        return_code = stan::services::sample::hmc_nuts_diag_e_adapt(
            model, num_chains, init_contexts, metric_contexts, random_seed, id,
            init_radius, num_warmup, num_samples, num_thin, save_warmup,
            refresh, stepsize, stepsize_jitter, max_depth, delta, gamma, kappa,
            t0, init_buffer, term_buffer, window, interrupt, logger,
            init_writers, sample_writers, diagnostic_writers);
      } else if (engine->value() == "nuts" && metric->value() == "unit_e"
                 && adapt_engaged == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("nuts"));
        int max_depth
            = dynamic_cast<int_argument *>(base->arg("max_depth"))->value();
        return_code = stan::services::sample::hmc_nuts_unit_e(
            model, *(init_contexts[0]), random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, interrupt, logger, init_writers[0],
            sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "nuts" && metric->value() == "unit_e"
                 && adapt_engaged == true) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("nuts"));
        int max_depth
            = dynamic_cast<int_argument *>(base->arg("max_depth"))->value();
        double delta
            = dynamic_cast<real_argument *>(adapt->arg("delta"))->value();
        double gamma
            = dynamic_cast<real_argument *>(adapt->arg("gamma"))->value();
        double kappa
            = dynamic_cast<real_argument *>(adapt->arg("kappa"))->value();
        double t0 = dynamic_cast<real_argument *>(adapt->arg("t0"))->value();
        return_code = stan::services::sample::hmc_nuts_unit_e_adapt(
            model, *(init_contexts[0]), random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, delta, gamma, kappa, t0, interrupt,
            logger, init_writers[0], sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "static" && metric->value() == "dense_e"
                 && adapt_engaged == false && metric_supplied == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        return_code = stan::services::sample::hmc_static_dense_e(
            model, *(init_contexts[0]), random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, interrupt, logger, init_writers[0],
            sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "static" && metric->value() == "dense_e"
                 && adapt_engaged == false && metric_supplied == true) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        return_code = stan::services::sample::hmc_static_dense_e(
            model, *(init_contexts[0]), *(metric_contexts[0]), random_seed, id,
            init_radius, num_warmup, num_samples, num_thin, save_warmup,
            refresh, stepsize, stepsize_jitter, int_time, interrupt, logger,
            init_writers[0], sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "static" && metric->value() == "dense_e"
                 && adapt_engaged == true && metric_supplied == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        double delta
            = dynamic_cast<real_argument *>(adapt->arg("delta"))->value();
        double gamma
            = dynamic_cast<real_argument *>(adapt->arg("gamma"))->value();
        double kappa
            = dynamic_cast<real_argument *>(adapt->arg("kappa"))->value();
        double t0 = dynamic_cast<real_argument *>(adapt->arg("t0"))->value();
        unsigned int init_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("init_buffer"))
                  ->value();
        unsigned int term_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("term_buffer"))
                  ->value();
        unsigned int window
            = dynamic_cast<u_int_argument *>(adapt->arg("window"))->value();
        return_code = stan::services::sample::hmc_static_dense_e_adapt(
            model, *(init_contexts[0]), random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, delta, gamma, kappa, t0, init_buffer,
            term_buffer, window, interrupt, logger, init_writers[0],
            sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "static" && metric->value() == "dense_e"
                 && adapt_engaged == true && metric_supplied == true) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        double delta
            = dynamic_cast<real_argument *>(adapt->arg("delta"))->value();
        double gamma
            = dynamic_cast<real_argument *>(adapt->arg("gamma"))->value();
        double kappa
            = dynamic_cast<real_argument *>(adapt->arg("kappa"))->value();
        double t0 = dynamic_cast<real_argument *>(adapt->arg("t0"))->value();
        unsigned int init_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("init_buffer"))
                  ->value();
        unsigned int term_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("term_buffer"))
                  ->value();
        unsigned int window
            = dynamic_cast<u_int_argument *>(adapt->arg("window"))->value();
        return_code = stan::services::sample::hmc_static_dense_e_adapt(
            model, *(init_contexts[0]), *(metric_contexts[0]), random_seed, id,
            init_radius, num_warmup, num_samples, num_thin, save_warmup,
            refresh, stepsize, stepsize_jitter, int_time, delta, gamma, kappa,
            t0, init_buffer, term_buffer, window, interrupt, logger,
            init_writers[0], sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "static" && metric->value() == "diag_e"
                 && adapt_engaged == false && metric_supplied == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        return_code = stan::services::sample::hmc_static_diag_e(
            model, *(init_contexts[0]), random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, interrupt, logger, init_writers[0],
            sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "static" && metric->value() == "diag_e"
                 && adapt_engaged == false && metric_supplied == true) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        return_code = stan::services::sample::hmc_static_diag_e(
            model, *(init_contexts[0]), *(metric_contexts[0]), random_seed, id,
            init_radius, num_warmup, num_samples, num_thin, save_warmup,
            refresh, stepsize, stepsize_jitter, int_time, interrupt, logger,
            init_writers[0], sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "static" && metric->value() == "diag_e"
                 && adapt_engaged == true && metric_supplied == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        double delta
            = dynamic_cast<real_argument *>(adapt->arg("delta"))->value();
        double gamma
            = dynamic_cast<real_argument *>(adapt->arg("gamma"))->value();
        double kappa
            = dynamic_cast<real_argument *>(adapt->arg("kappa"))->value();
        double t0 = dynamic_cast<real_argument *>(adapt->arg("t0"))->value();
        unsigned int init_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("init_buffer"))
                  ->value();
        unsigned int term_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("term_buffer"))
                  ->value();
        unsigned int window
            = dynamic_cast<u_int_argument *>(adapt->arg("window"))->value();
        return_code = stan::services::sample::hmc_static_diag_e_adapt(
            model, *(init_contexts[0]), random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, delta, gamma, kappa, t0, init_buffer,
            term_buffer, window, interrupt, logger, init_writers[0],
            sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "static" && metric->value() == "diag_e"
                 && adapt_engaged == true && metric_supplied == true) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        double delta
            = dynamic_cast<real_argument *>(adapt->arg("delta"))->value();
        double gamma
            = dynamic_cast<real_argument *>(adapt->arg("gamma"))->value();
        double kappa
            = dynamic_cast<real_argument *>(adapt->arg("kappa"))->value();
        double t0 = dynamic_cast<real_argument *>(adapt->arg("t0"))->value();
        unsigned int init_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("init_buffer"))
                  ->value();
        unsigned int term_buffer
            = dynamic_cast<u_int_argument *>(adapt->arg("term_buffer"))
                  ->value();
        unsigned int window
            = dynamic_cast<u_int_argument *>(adapt->arg("window"))->value();
        return_code = stan::services::sample::hmc_static_diag_e_adapt(
            model, *(init_contexts[0]), *(metric_contexts[0]), random_seed, id,
            init_radius, num_warmup, num_samples, num_thin, save_warmup,
            refresh, stepsize, stepsize_jitter, int_time, delta, gamma, kappa,
            t0, init_buffer, term_buffer, window, interrupt, logger,
            init_writers[0], sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "static" && metric->value() == "unit_e"
                 && adapt_engaged == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        return_code = stan::services::sample::hmc_static_unit_e(
            model, *(init_contexts[0]), random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, interrupt, logger, init_writers[0],
            sample_writers[0], diagnostic_writers[0]);
      } else if (engine->value() == "static" && metric->value() == "unit_e"
                 && adapt_engaged == true) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        double delta
            = dynamic_cast<real_argument *>(adapt->arg("delta"))->value();
        double gamma
            = dynamic_cast<real_argument *>(adapt->arg("gamma"))->value();
        double kappa
            = dynamic_cast<real_argument *>(adapt->arg("kappa"))->value();
        double t0 = dynamic_cast<real_argument *>(adapt->arg("t0"))->value();
        return_code = stan::services::sample::hmc_static_unit_e_adapt(
            model, *(init_contexts[0]), random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, delta, gamma, kappa, t0, interrupt,
            logger, init_writers[0], sample_writers[0], diagnostic_writers[0]);
      }
    }
  } else if (user_method->arg("variational")) {
    list_argument *algo = dynamic_cast<list_argument *>(
        parser.arg("method")->arg("variational")->arg("algorithm"));
    int grad_samples
        = dynamic_cast<int_argument *>(
              parser.arg("method")->arg("variational")->arg("grad_samples"))
              ->value();
    int elbo_samples
        = dynamic_cast<int_argument *>(
              parser.arg("method")->arg("variational")->arg("elbo_samples"))
              ->value();
    int max_iterations
        = dynamic_cast<int_argument *>(
              parser.arg("method")->arg("variational")->arg("iter"))
              ->value();
    double tol_rel_obj
        = dynamic_cast<real_argument *>(
              parser.arg("method")->arg("variational")->arg("tol_rel_obj"))
              ->value();
    double eta = dynamic_cast<real_argument *>(
                     parser.arg("method")->arg("variational")->arg("eta"))
                     ->value();
    bool adapt_engaged = dynamic_cast<bool_argument *>(parser.arg("method")
                                                           ->arg("variational")
                                                           ->arg("adapt")
                                                           ->arg("engaged"))
                             ->value();
    int adapt_iterations = dynamic_cast<int_argument *>(parser.arg("method")
                                                            ->arg("variational")
                                                            ->arg("adapt")
                                                            ->arg("iter"))
                               ->value();
    int eval_elbo
        = dynamic_cast<int_argument *>(
              parser.arg("method")->arg("variational")->arg("eval_elbo"))
              ->value();
    int output_samples
        = dynamic_cast<int_argument *>(
              parser.arg("method")->arg("variational")->arg("output_samples"))
              ->value();

    if (algo->value() == "fullrank") {
      return_code = stan::services::experimental::advi::fullrank(
          model, *(init_contexts[0]), random_seed, id, init_radius,
          grad_samples, elbo_samples, max_iterations, tol_rel_obj, eta,
          adapt_engaged, adapt_iterations, eval_elbo, output_samples, interrupt,
          logger, init_writers[0], sample_writers[0], diagnostic_writers[0]);
    } else if (algo->value() == "meanfield") {
      return_code = stan::services::experimental::advi::meanfield(
          model, *(init_contexts[0]), random_seed, id, init_radius,
          grad_samples, elbo_samples, max_iterations, tol_rel_obj, eta,
          adapt_engaged, adapt_iterations, eval_elbo, output_samples, interrupt,
          logger, init_writers[0], sample_writers[0], diagnostic_writers[0]);
    }
  }
  stan::math::profile_map &profile_data = get_stan_profile_data();
  if (profile_data.size() > 0) {
    std::string profile_file_name
        = dynamic_cast<string_argument *>(
              parser.arg("output")->arg("profile_file"))
              ->value();
    std::fstream profile_stream(profile_file_name.c_str(), std::fstream::out);
    if (!sig_figs_arg->is_default()) {
      profile_stream << std::setprecision(sig_figs_arg->value());
    }
    write_profiling(profile_stream, profile_data);
    profile_stream.close();
  }
  for (size_t i = 0; i < valid_arguments.size(); ++i) {
    delete valid_arguments.at(i);
  }
#ifdef STAN_MPI
  cluster.stop_listen();
#endif
  return return_code;
}

}  // namespace cmdstan
#endif
