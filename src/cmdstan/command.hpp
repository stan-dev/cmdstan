#ifndef CMDSTAN_COMMAND_HPP
#define CMDSTAN_COMMAND_HPP

#include <cmdstan/arguments/arg_data.hpp>
#include <cmdstan/arguments/arg_id.hpp>
#include <cmdstan/arguments/arg_init.hpp>
#include <cmdstan/arguments/arg_output.hpp>
#include <cmdstan/arguments/arg_random.hpp>
#include <cmdstan/arguments/arg_opencl.hpp>
#include <cmdstan/arguments/arg_profile_file.hpp>
#include <cmdstan/arguments/argument_parser.hpp>
#include <cmdstan/io/json/json_data.hpp>
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
#include <stan/callbacks/stream_writer.hpp>
#include <stan/callbacks/writer.hpp>
#include <stan/io/dump.hpp>
#include <stan/io/ends_with.hpp>
#include <stan/io/stan_csv_reader.hpp>
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

std::shared_ptr<stan::io::var_context> get_var_context(const std::string file) {
  std::fstream stream(file.c_str(), std::fstream::in);
  if (file != "" && (stream.rdstate() & std::ifstream::failbit)) {
    std::stringstream msg;
    msg << "Can't open specified file, \"" << file << "\"" << std::endl;
    throw std::invalid_argument(msg.str());
  }
  if (stan::io::ends_with(".json", file)) {
    cmdstan::json::json_data var_context(stream);
    stream.close();
    std::shared_ptr<stan::io::var_context> result
        = std::make_shared<cmdstan::json::json_data>(var_context);
    return result;
  }
  stan::io::dump var_context(stream);
  stream.close();
  std::shared_ptr<stan::io::var_context> result
      = std::make_shared<stan::io::dump>(var_context);
  return result;
}

static int hmc_fixed_cols = 7;  // hmc sampler outputs columns __lp + 6

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

  stan::math::init_threadpool_tbb();

  // Read arguments
  std::vector<argument *> valid_arguments;
  valid_arguments.push_back(new arg_id());
  valid_arguments.push_back(new arg_data());
  valid_arguments.push_back(new arg_init());
  valid_arguments.push_back(new arg_random());
  valid_arguments.push_back(new arg_output());
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

  std::fstream output_stream(
      dynamic_cast<string_argument *>(parser.arg("output")->arg("file"))
          ->value()
          .c_str(),
      std::fstream::out);

  int_argument *sig_figs_arg
      = dynamic_cast<int_argument *>(parser.arg("output")->arg("sig_figs"));
  if (!sig_figs_arg->is_default()) {
    output_stream << std::setprecision(sig_figs_arg->value());
  }
  stan::callbacks::stream_writer sample_writer(output_stream, "# ");

  std::fstream diagnostic_stream(
      dynamic_cast<string_argument *>(
          parser.arg("output")->arg("diagnostic_file"))
          ->value()
          .c_str(),
      std::fstream::out);
  stan::callbacks::stream_writer diagnostic_writer(diagnostic_stream, "# ");

  // Read input data
  std::string filename(
      dynamic_cast<string_argument *>(parser.arg("data")->arg("file"))
          ->value());
  std::shared_ptr<stan::io::var_context> var_context
      = get_var_context(filename);

  // Instantiate model
  stan::model::model_base &model
      = new_model(*var_context, random_seed, &std::cout);

  std::vector<std::string> model_compile_info = model.model_compile_info();

  write_stan(sample_writer);
  write_model(sample_writer, model.model_name());
  write_datetime(sample_writer);
  parser.print(sample_writer);
  write_parallel_info(sample_writer);
  write_opencl_device(sample_writer);
  write_compile_info(sample_writer, model_compile_info);

  write_stan(diagnostic_writer);
  write_model(diagnostic_writer, model.model_name());
  parser.print(diagnostic_writer);

  int refresh
      = dynamic_cast<int_argument *>(parser.arg("output")->arg("refresh"))
            ->value();
  unsigned int id = dynamic_cast<int_argument *>(parser.arg("id"))->value();

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
  std::shared_ptr<stan::io::var_context> init_context = get_var_context(init);

  // Invoke specified method
  int return_code = return_codes::OK;

  if (parser.arg("method")->arg("generate_quantities")) {
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
    size_t num_cols = param_names.size();
    size_t num_rows = fitted_params.samples.rows();
    // check that all parameter names are in sample, in order
    if (num_cols + hmc_fixed_cols > fitted_params.header.size()) {
      msg << "Mismatch between model and fitted_parameters csv file \"" << fname
          << "\"" << std::endl;
      throw std::invalid_argument(msg.str());
    }
    for (size_t i = 0; i < num_cols; ++i) {
      if (param_names[i].compare(fitted_params.header[i + hmc_fixed_cols])
          != 0) {
        msg << "Mismatch between model and fitted_parameters csv file \""
            << fname << "\"" << std::endl;
        throw std::invalid_argument(msg.str());
      }
    }
    return_code = stan::services::standalone_generate(
        model,
        fitted_params.samples.block(0, hmc_fixed_cols, num_rows, num_cols),
        random_seed, interrupt, logger, sample_writer);
  } else if (parser.arg("method")->arg("diagnose")) {
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
          model, *init_context, random_seed, id, init_radius, epsilon, error,
          interrupt, logger, init_writer, sample_writer);
    }
  } else if (parser.arg("method")->arg("optimize")) {
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
          model, *init_context, random_seed, id, init_radius, num_iterations,
          save_iterations, interrupt, logger, init_writer, sample_writer);
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
          model, *init_context, random_seed, id, init_radius, init_alpha,
          tol_obj, tol_rel_obj, tol_grad, tol_rel_grad, tol_param,
          num_iterations, save_iterations, refresh, interrupt, logger,
          init_writer, sample_writer);
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
          model, *init_context, random_seed, id, init_radius, history_size,
          init_alpha, tol_obj, tol_rel_obj, tol_grad, tol_rel_grad, tol_param,
          num_iterations, save_iterations, refresh, interrupt, logger,
          init_writer, sample_writer);
    }
  } else if (parser.arg("method")->arg("sample")) {
    int num_warmup = dynamic_cast<int_argument *>(
                         parser.arg("method")->arg("sample")->arg("num_warmup"))
                         ->value();
    int num_samples
        = dynamic_cast<int_argument *>(
              parser.arg("method")->arg("sample")->arg("num_samples"))
              ->value();
    int num_thin = dynamic_cast<int_argument *>(
                       parser.arg("method")->arg("sample")->arg("thin"))
                       ->value();
    bool save_warmup
        = dynamic_cast<bool_argument *>(
              parser.arg("method")->arg("sample")->arg("save_warmup"))
              ->value();
    list_argument *algo = dynamic_cast<list_argument *>(
        parser.arg("method")->arg("sample")->arg("algorithm"));
    categorical_argument *adapt = dynamic_cast<categorical_argument *>(
        parser.arg("method")->arg("sample")->arg("adapt"));
    bool adapt_engaged
        = dynamic_cast<bool_argument *>(adapt->arg("engaged"))->value();

    if (model.num_params_r() == 0 || algo->value() == "fixed_param") {
      if (algo->value() != "fixed_param")
        info(
            "Model contains no parameters, running fixed_param sampler, "
            "no updates to Markov chain");
      return_code = stan::services::sample::fixed_param(
          model, *init_context, random_seed, id, init_radius, num_samples,
          num_thin, refresh, interrupt, logger, init_writer, sample_writer,
          diagnostic_writer);
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
      std::shared_ptr<stan::io::var_context> metric_context
          = get_var_context(metric_filename);

      categorical_argument *adapt = dynamic_cast<categorical_argument *>(
          parser.arg("method")->arg("sample")->arg("adapt"));
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
            model, *init_context, random_seed, id, init_radius, num_warmup,
            num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, interrupt, logger, init_writer,
            sample_writer, diagnostic_writer);
      } else if (engine->value() == "nuts" && metric->value() == "dense_e"
                 && adapt_engaged == false && metric_supplied == true) {
        int max_depth = dynamic_cast<int_argument *>(
                            dynamic_cast<categorical_argument *>(
                                algo->arg("hmc")->arg("engine")->arg("nuts"))
                                ->arg("max_depth"))
                            ->value();
        return_code = stan::services::sample::hmc_nuts_dense_e(
            model, *init_context, *metric_context, random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, interrupt, logger, init_writer,
            sample_writer, diagnostic_writer);
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
            model, *init_context, random_seed, id, init_radius, num_warmup,
            num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, delta, gamma, kappa, t0, init_buffer,
            term_buffer, window, interrupt, logger, init_writer, sample_writer,
            diagnostic_writer);
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
            model, *init_context, *metric_context, random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, delta, gamma, kappa, t0, init_buffer,
            term_buffer, window, interrupt, logger, init_writer, sample_writer,
            diagnostic_writer);
      } else if (engine->value() == "nuts" && metric->value() == "diag_e"
                 && adapt_engaged == false && metric_supplied == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("nuts"));
        int max_depth
            = dynamic_cast<int_argument *>(base->arg("max_depth"))->value();
        return_code = stan::services::sample::hmc_nuts_diag_e(
            model, *init_context, random_seed, id, init_radius, num_warmup,
            num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, interrupt, logger, init_writer,
            sample_writer, diagnostic_writer);
      } else if (engine->value() == "nuts" && metric->value() == "diag_e"
                 && adapt_engaged == false && metric_supplied == true) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("nuts"));
        int max_depth
            = dynamic_cast<int_argument *>(base->arg("max_depth"))->value();
        return_code = stan::services::sample::hmc_nuts_diag_e(
            model, *init_context, *metric_context, random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, interrupt, logger, init_writer,
            sample_writer, diagnostic_writer);
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
            model, *init_context, random_seed, id, init_radius, num_warmup,
            num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, delta, gamma, kappa, t0, init_buffer,
            term_buffer, window, interrupt, logger, init_writer, sample_writer,
            diagnostic_writer);
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
            model, *init_context, *metric_context, random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, delta, gamma, kappa, t0, init_buffer,
            term_buffer, window, interrupt, logger, init_writer, sample_writer,
            diagnostic_writer);
      } else if (engine->value() == "nuts" && metric->value() == "unit_e"
                 && adapt_engaged == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("nuts"));
        int max_depth
            = dynamic_cast<int_argument *>(base->arg("max_depth"))->value();
        return_code = stan::services::sample::hmc_nuts_unit_e(
            model, *init_context, random_seed, id, init_radius, num_warmup,
            num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, interrupt, logger, init_writer,
            sample_writer, diagnostic_writer);
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
            model, *init_context, random_seed, id, init_radius, num_warmup,
            num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, max_depth, delta, gamma, kappa, t0, interrupt,
            logger, init_writer, sample_writer, diagnostic_writer);
      } else if (engine->value() == "static" && metric->value() == "dense_e"
                 && adapt_engaged == false && metric_supplied == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        return_code = stan::services::sample::hmc_static_dense_e(
            model, *init_context, random_seed, id, init_radius, num_warmup,
            num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, interrupt, logger, init_writer,
            sample_writer, diagnostic_writer);
      } else if (engine->value() == "static" && metric->value() == "dense_e"
                 && adapt_engaged == false && metric_supplied == true) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        return_code = stan::services::sample::hmc_static_dense_e(
            model, *init_context, *metric_context, random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, interrupt, logger, init_writer,
            sample_writer, diagnostic_writer);
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
            model, *init_context, random_seed, id, init_radius, num_warmup,
            num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, delta, gamma, kappa, t0, init_buffer,
            term_buffer, window, interrupt, logger, init_writer, sample_writer,
            diagnostic_writer);
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
            model, *init_context, *metric_context, random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, delta, gamma, kappa, t0, init_buffer,
            term_buffer, window, interrupt, logger, init_writer, sample_writer,
            diagnostic_writer);
      } else if (engine->value() == "static" && metric->value() == "diag_e"
                 && adapt_engaged == false && metric_supplied == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        return_code = stan::services::sample::hmc_static_diag_e(
            model, *init_context, random_seed, id, init_radius, num_warmup,
            num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, interrupt, logger, init_writer,
            sample_writer, diagnostic_writer);
      } else if (engine->value() == "static" && metric->value() == "diag_e"
                 && adapt_engaged == false && metric_supplied == true) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        return_code = stan::services::sample::hmc_static_diag_e(
            model, *init_context, *metric_context, random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, interrupt, logger, init_writer,
            sample_writer, diagnostic_writer);
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
            model, *init_context, random_seed, id, init_radius, num_warmup,
            num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, delta, gamma, kappa, t0, init_buffer,
            term_buffer, window, interrupt, logger, init_writer, sample_writer,
            diagnostic_writer);
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
            model, *init_context, *metric_context, random_seed, id, init_radius,
            num_warmup, num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, delta, gamma, kappa, t0, init_buffer,
            term_buffer, window, interrupt, logger, init_writer, sample_writer,
            diagnostic_writer);
      } else if (engine->value() == "static" && metric->value() == "unit_e"
                 && adapt_engaged == false) {
        categorical_argument *base = dynamic_cast<categorical_argument *>(
            algo->arg("hmc")->arg("engine")->arg("static"));
        double int_time
            = dynamic_cast<real_argument *>(base->arg("int_time"))->value();
        return_code = stan::services::sample::hmc_static_unit_e(
            model, *init_context, random_seed, id, init_radius, num_warmup,
            num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, interrupt, logger, init_writer,
            sample_writer, diagnostic_writer);
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
            model, *init_context, random_seed, id, init_radius, num_warmup,
            num_samples, num_thin, save_warmup, refresh, stepsize,
            stepsize_jitter, int_time, delta, gamma, kappa, t0, interrupt,
            logger, init_writer, sample_writer, diagnostic_writer);
      }
    }
  } else if (parser.arg("method")->arg("variational")) {
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
          model, *init_context, random_seed, id, init_radius, grad_samples,
          elbo_samples, max_iterations, tol_rel_obj, eta, adapt_engaged,
          adapt_iterations, eval_elbo, output_samples, interrupt, logger,
          init_writer, sample_writer, diagnostic_writer);
    } else if (algo->value() == "meanfield") {
      return_code = stan::services::experimental::advi::meanfield(
          model, *init_context, random_seed, id, init_radius, grad_samples,
          elbo_samples, max_iterations, tol_rel_obj, eta, adapt_engaged,
          adapt_iterations, eval_elbo, output_samples, interrupt, logger,
          init_writer, sample_writer, diagnostic_writer);
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
  output_stream.close();
  diagnostic_stream.close();
  for (size_t i = 0; i < valid_arguments.size(); ++i)
    delete valid_arguments.at(i);
#ifdef STAN_MPI
  cluster.stop_listen();
#endif
  return return_code;
}

}  // namespace cmdstan
#endif
