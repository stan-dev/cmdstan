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
#include <cmdstan/command_helper.hpp>
#include <cmdstan/return_codes.hpp>
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
#include <stan/callbacks/json_writer.hpp>
#include <stan/callbacks/logger.hpp>
#include <stan/callbacks/stream_logger.hpp>
#include <stan/callbacks/stream_writer.hpp>
#include <stan/callbacks/unique_stream_writer.hpp>
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
#include <stan/services/optimize/laplace_sample.hpp>
#include <stan/services/optimize/newton.hpp>
#include <stan/services/pathfinder/multi.hpp>
#include <stan/services/pathfinder/single.hpp>
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

#ifdef STAN_MPI
stan::math::mpi_cluster &get_mpi_cluster() {
  static stan::math::mpi_cluster cluster;
  return cluster;
}
#endif

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

#ifdef STAN_OPENCL
  int opencl_device_id = get_arg_val<int_argument>(parser, "opencl", "device");
  int opencl_platform_id
      = get_arg_val<int_argument>(parser, "opencl", "platform");
  if ((opencl_device_id >= 0 && opencl_platform_id < 0)  // default value -1
      || (opencl_device_id < 0 && opencl_platform_id >= 0)) {
    std::cerr << "Please set both device and platform OpenCL IDs." << std::endl;
    return return_codes::NOT_OK;
  } else if (opencl_device_id >= 0) {  // opencl_platform_id >= 0 by above test
    stan::math::opencl_context.select_device(opencl_platform_id,
                                             opencl_device_id);
  }
#endif

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

  unsigned int num_chains = get_num_chains(parser);
  check_file_config(parser);

  parser.print(info);
  write_parallel_info(info);
  write_opencl_device(info);
  info();

  // General config for all methods
  unsigned int id = get_arg_val<int_argument>(parser, "id");
  int sig_figs = get_arg_val<int_argument>(parser, "output", "sig_figs");
  int refresh = get_arg_val<int_argument>(parser, "output", "refresh");

  //////////////////////////////////////////////////
  //                Initialize Model              //
  //////////////////////////////////////////////////
  arg_seed *random_arg
      = dynamic_cast<arg_seed *>(parser.arg("random")->arg("seed"));
  unsigned int random_seed = random_arg->random_value();

  std::string filename = get_arg_val<string_argument>(parser, "data", "file");

  std::shared_ptr<stan::io::var_context> var_context
      = get_var_context(filename);

  stan::model::model_base &model
      = new_model(*var_context, random_seed, &std::cout);

  // Setup callbacks
  stan::callbacks::interrupt interrupt;
  stan::callbacks::json_writer<std::ofstream> dummy_json_writer;
  stan::callbacks::writer init_writer;  // always no-op writer
  std::vector<stan::callbacks::writer> init_writers{num_chains,
                                                    stan::callbacks::writer{}};
  std::vector<stan::callbacks::unique_stream_writer<std::ofstream>>
      sample_writers;
  std::vector<stan::callbacks::unique_stream_writer<std::ofstream>>
      diagnostic_csv_writers;
  std::vector<stan::callbacks::json_writer<std::ofstream>>
      diagnostic_json_writers;
  init_callbacks(parser, sample_writers, diagnostic_csv_writers,
                 diagnostic_json_writers);

  // Setup initial parameter values - arg "init"
  // arg is either filename or init radius value
  std::string init = get_arg_val<string_argument>(parser, "init");
  double init_radius = 2.0;
  try {
    init_radius = std::stod(init);
    init = "";
  } catch (const std::exception &e) {
  }
  std::vector<std::shared_ptr<stan::io::var_context>> init_contexts
      = get_vec_var_context(init, num_chains);

  std::vector<std::string> model_compile_info = model.model_compile_info();
  for (int i = 0; i < num_chains; ++i) {
    write_stan(sample_writers[i]);
    write_model(sample_writers[i], model.model_name());
    write_datetime(sample_writers[i]);
    parser.print(sample_writers[i]);
    write_parallel_info(sample_writers[i]);
    write_opencl_device(sample_writers[i]);
    write_compile_info(sample_writers[i], model_compile_info);
    write_stan(diagnostic_csv_writers[i]);
    write_model(diagnostic_csv_writers[i], model.model_name());
    parser.print(diagnostic_csv_writers[i]);
  }

  //////////////////////////////////////////////////
  //            Invoke Services                   //
  //////////////////////////////////////////////////
  int return_code = return_codes::NOT_OK;
  std::stringstream msg;
  auto user_method = parser.arg("method");

  if (user_method->arg("pathfinder")) {
    // ---- pathfinder start ---- //
    auto pathfinder_arg = parser.arg("method")->arg("pathfinder");
    int history_size
        = get_arg_val<int_argument>(*pathfinder_arg, "history_size");
    double init_alpha
        = get_arg_val<real_argument>(*pathfinder_arg, "init_alpha");
    double tol_obj = get_arg_val<real_argument>(*pathfinder_arg, "tol_obj");
    double tol_rel_obj
        = get_arg_val<real_argument>(*pathfinder_arg, "tol_rel_obj");
    double tol_grad = get_arg_val<real_argument>(*pathfinder_arg, "tol_grad");
    double tol_rel_grad
        = get_arg_val<real_argument>(*pathfinder_arg, "tol_rel_grad");
    double tol_param = get_arg_val<real_argument>(*pathfinder_arg, "tol_param");
    int max_lbfgs_iters
        = get_arg_val<int_argument>(*pathfinder_arg, "max_lbfgs_iters");
    int num_elbo_draws
        = get_arg_val<int_argument>(*pathfinder_arg, "num_elbo_draws");
    int num_draws = get_arg_val<int_argument>(*pathfinder_arg, "num_draws");
    int num_psis_draws
        = get_arg_val<int_argument>(*pathfinder_arg, "num_psis_draws");
    int num_paths = get_arg_val<int_argument>(*pathfinder_arg, "num_paths");
    bool save_iterations
        = !get_arg_val<string_argument>(parser, "output", "diagnostic_file")
               .empty();
    if (num_paths == 1) {
      return_code = stan::services::pathfinder::pathfinder_lbfgs_single<
          false, stan::model::model_base>(
          model, *(init_contexts[0]), random_seed, id, init_radius,
          history_size, init_alpha, tol_obj, tol_rel_obj, tol_grad,
          tol_rel_grad, tol_param, max_lbfgs_iters, num_elbo_draws, num_draws,
          save_iterations, refresh, interrupt, logger, init_writer,
          sample_writers[0], diagnostic_json_writers[0]);
    } else {
      std::string output_file
          = get_arg_val<string_argument>(parser, "output", "file");
      std::string pf_name;
      std::string pf_suffix;
      get_basename_suffix(output_file, pf_name, pf_suffix);
      if (pf_suffix.empty())
        pf_suffix = ".csv";
      auto ofs = std::make_unique<std::ofstream>(pf_name + "_pathfinder"
                                                 + pf_suffix);
      if (sig_figs > -1)
        ofs->precision(sig_figs);
      stan::callbacks::unique_stream_writer<std::ofstream> pathfinder_writer(
          std::move(ofs), "# ");
      write_stan(pathfinder_writer);
      write_model(pathfinder_writer, model.model_name());
      write_datetime(pathfinder_writer);
      parser.print(pathfinder_writer);
      return_code = stan::services::pathfinder::pathfinder_lbfgs_multi<
          stan::model::model_base>(
          model, init_contexts, random_seed, id, init_radius, history_size,
          init_alpha, tol_obj, tol_rel_obj, tol_grad, tol_rel_grad, tol_param,
          max_lbfgs_iters, num_elbo_draws, num_draws, num_psis_draws, num_paths,
          save_iterations, refresh, interrupt, logger, init_writers,
          sample_writers, diagnostic_json_writers, pathfinder_writer,
          dummy_json_writer);
    }
    // ---- pathfinder end ---- //
  } else if (user_method->arg("generate_quantities")) {
    // ---- generate_quantities start ---- //
    auto gq_arg = parser.arg("method")->arg("generate_quantities");
    std::string fname = get_arg_val<string_argument>(*gq_arg, "fitted_params");
    if (fname.empty()) {
      msg << "Missing fitted_params argument, cannot run generate_quantities "
             "without fitted sample.";
      throw std::invalid_argument(msg.str());
    }
    std::vector<std::string> param_names = get_constrained_param_names(model);
    stan::io::stan_csv fitted_params;
    size_t col_offset, num_rows, num_cols;
    parse_stan_csv(fname, model, param_names, fitted_params, col_offset,
                   num_rows, num_cols);
    return_code = stan::services::standalone_generate(
        model, fitted_params.samples.block(0, col_offset, num_rows, num_cols),
        random_seed, interrupt, logger, sample_writers[0]);
    // ---- generate_quantities end ---- //
  } else if (user_method->arg("laplace")) {
    // ---- laplace start ---- //
    auto laplace_arg = parser.arg("method")->arg("laplace");
    std::string fname = get_arg_val<string_argument>(*laplace_arg, "mode");
    if (fname.empty()) {
      msg << "Missing mode argument, cannot get laplace sample "
             "without parameter estimates theta-hat";
      throw std::invalid_argument(msg.str());
    }
    Eigen::VectorXd theta_hat = get_laplace_mode(fname, model);
    bool jacobian = get_arg_val<bool_argument>(*laplace_arg, "jacobian");
    int draws = get_arg_val<int_argument>(*laplace_arg, "draws");
    if (jacobian) {
      return_code = stan::services::laplace_sample<true>(
          model, theta_hat, draws, random_seed, refresh, interrupt, logger,
          sample_writers[0]);
    } else {
      return_code = stan::services::laplace_sample<false>(
          model, theta_hat, draws, random_seed, refresh, interrupt, logger,
          sample_writers[0]);
    }
    // ---- laplace end ---- //
  } else if (user_method->arg("log_prob")) {
    // ---- log_prob start ---- //
    auto log_prob_arg = parser.arg("method")->arg("log_prob");
    std::string upars_file
        = get_arg_val<string_argument>(*log_prob_arg, "unconstrained_params");
    std::string cpars_file
        = get_arg_val<string_argument>(*log_prob_arg, "constrained_params");
    bool jacobian = get_arg_val<bool_argument>(*log_prob_arg, "jacobian");
    if (upars_file.length() == 0 && cpars_file.length() == 0) {
      msg << "No input parameter files provided, "
          << "cannot calculate log probability density.";
      throw std::invalid_argument(msg.str());
    }
    if (upars_file.length() > 0 && cpars_file.length() > 0) {
      msg << "Cannot specify both input files of both "
          << "constrained and unconstrained parameter values.";
      throw std::invalid_argument(msg.str());
    }
    std::vector<std::vector<double>> params_r_ind;
    if (upars_file.length() > 0) {
      params_r_ind = get_uparams_r(upars_file, model);
    } else if (cpars_file.length() > 0) {
      std::vector<std::string> param_names = get_constrained_param_names(model);
      if (get_suffix(cpars_file) == ".csv") {
        stan::io::stan_csv fitted_params;
        size_t col_offset, num_rows, num_cols;
        parse_stan_csv(cpars_file, model, param_names, fitted_params,
                       col_offset, num_rows, num_cols);
        params_r_ind = unconstrain_params_csv(model, fitted_params, col_offset,
                                              num_rows, num_cols);
      } else {
        params_r_ind = {unconstrain_params_var_context(cpars_file, model)};
      }
    }
    try {
      services_log_prob_grad(model, jacobian, params_r_ind, sig_figs,
                             sample_writers[0].get_stream());
      return_code = return_codes::OK;
    } catch (const std::exception &e) {
      return_code = return_codes::NOT_OK;
    }
    // ---- log_prob end ---- //
  } else if (user_method->arg("diagnose")) {
    // ---- diagnose start ---- //
    list_argument *test = dynamic_cast<list_argument *>(
        parser.arg("method")->arg("diagnose")->arg("test"));
    if (test->value() == "gradient") {
      double epsilon = get_arg_val<real_argument>(*test, "gradient", "epsilon");
      double error = get_arg_val<real_argument>(*test, "gradient", "error");
      return_code = stan::services::diagnose::diagnose(
          model, *(init_contexts[0]), random_seed, id, init_radius, epsilon,
          error, interrupt, logger, init_writers[0], sample_writers[0]);
    }
    // ---- diagnose end ---- //
  } else if (user_method->arg("optimize")) {
    // ---- optimize start ---- //
    int num_iterations
        = get_arg_val<int_argument>(parser, "method", "optimize", "iter");
    bool save_iterations = get_arg_val<bool_argument>(
        parser, "method", "optimize", "save_iterations");
    bool jacobian
        = get_arg_val<bool_argument>(parser, "method", "optimize", "jacobian");
    list_argument *algo = dynamic_cast<list_argument *>(
        parser.arg("method")->arg("optimize")->arg("algorithm"));
    if (algo->value() == "newton") {
      if (jacobian) {
        return_code
            = stan::services::optimize::newton<stan::model::model_base, true>(
                model, *(init_contexts[0]), random_seed, id, init_radius,
                num_iterations, save_iterations, interrupt, logger,
                init_writers[0], sample_writers[0]);
      } else {
        return_code
            = stan::services::optimize::newton<stan::model::model_base, false>(
                model, *(init_contexts[0]), random_seed, id, init_radius,
                num_iterations, save_iterations, interrupt, logger,
                init_writers[0], sample_writers[0]);
      }
    } else if (algo->value() == "bfgs") {
      double init_alpha
          = get_arg_val<real_argument>(*algo, "bfgs", "init_alpha");
      double tol_obj = get_arg_val<real_argument>(*algo, "bfgs", "tol_obj");
      double tol_rel_obj
          = get_arg_val<real_argument>(*algo, "bfgs", "tol_rel_obj");
      double tol_grad = get_arg_val<real_argument>(*algo, "bfgs", "tol_grad");
      double tol_rel_grad
          = get_arg_val<real_argument>(*algo, "bfgs", "tol_rel_grad");
      double tol_param = get_arg_val<real_argument>(*algo, "bfgs", "tol_param");

      if (jacobian) {
        return_code
            = stan::services::optimize::bfgs<stan::model::model_base, true>(
                model, *(init_contexts[0]), random_seed, id, init_radius,
                init_alpha, tol_obj, tol_rel_obj, tol_grad, tol_rel_grad,
                tol_param, num_iterations, save_iterations, refresh, interrupt,
                logger, init_writers[0], sample_writers[0]);
      } else {
        return_code
            = stan::services::optimize::bfgs<stan::model::model_base, false>(
                model, *(init_contexts[0]), random_seed, id, init_radius,
                init_alpha, tol_obj, tol_rel_obj, tol_grad, tol_rel_grad,
                tol_param, num_iterations, save_iterations, refresh, interrupt,
                logger, init_writers[0], sample_writers[0]);
      }
    } else if (algo->value() == "lbfgs") {
      int history_size
          = get_arg_val<int_argument>(*algo, "lbfgs", "history_size");
      double init_alpha
          = get_arg_val<real_argument>(*algo, "lbfgs", "init_alpha");
      double tol_obj = get_arg_val<real_argument>(*algo, "lbfgs", "tol_obj");
      double tol_rel_obj
          = get_arg_val<real_argument>(*algo, "lbfgs", "tol_rel_obj");
      double tol_grad = get_arg_val<real_argument>(*algo, "lbfgs", "tol_grad");
      double tol_rel_grad
          = get_arg_val<real_argument>(*algo, "lbfgs", "tol_rel_grad");
      double tol_param
          = get_arg_val<real_argument>(*algo, "lbfgs", "tol_param");

      if (jacobian) {
        return_code
            = stan::services::optimize::lbfgs<stan::model::model_base, true>(
                model, *(init_contexts[0]), random_seed, id, init_radius,
                history_size, init_alpha, tol_obj, tol_rel_obj, tol_grad,
                tol_rel_grad, tol_param, num_iterations, save_iterations,
                refresh, interrupt, logger, init_writers[0], sample_writers[0]);
      } else {
        return_code
            = stan::services::optimize::lbfgs<stan::model::model_base, false>(
                model, *(init_contexts[0]), random_seed, id, init_radius,
                history_size, init_alpha, tol_obj, tol_rel_obj, tol_grad,
                tol_rel_grad, tol_param, num_iterations, save_iterations,
                refresh, interrupt, logger, init_writers[0], sample_writers[0]);
      }
    }
    // ---- optimize end ---- //
  } else if (user_method->arg("sample")) {
    // ---- sample start ---- //
    auto sample_arg = parser.arg("method")->arg("sample");
    int num_warmup
        = get_arg_val<int_argument>(parser, "method", "sample", "num_warmup");
    int num_samples
        = get_arg_val<int_argument>(parser, "method", "sample", "num_samples");
    int num_thin
        = get_arg_val<int_argument>(parser, "method", "sample", "thin");
    bool save_warmup
        = get_arg_val<bool_argument>(parser, "method", "sample", "save_warmup");
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
          sample_writers[0], diagnostic_csv_writers[0]);
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
        return_code = return_codes::NOT_OK;
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
            sample_writers[0], diagnostic_csv_writers[0]);
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
            init_writers[0], sample_writers[0], diagnostic_csv_writers[0]);
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
            sample_writers, diagnostic_csv_writers);
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
            init_writers, sample_writers, diagnostic_csv_writers);
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
            sample_writers[0], diagnostic_csv_writers[0]);
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
            init_writers[0], sample_writers[0], diagnostic_csv_writers[0]);
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
            sample_writers, diagnostic_csv_writers);
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
            init_writers, sample_writers, diagnostic_csv_writers);
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
            sample_writers[0], diagnostic_csv_writers[0]);
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
            logger, init_writers[0], sample_writers[0],
            diagnostic_csv_writers[0]);
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
            sample_writers[0], diagnostic_csv_writers[0]);
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
            init_writers[0], sample_writers[0], diagnostic_csv_writers[0]);
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
            sample_writers[0], diagnostic_csv_writers[0]);
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
            init_writers[0], sample_writers[0], diagnostic_csv_writers[0]);
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
            sample_writers[0], diagnostic_csv_writers[0]);
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
            init_writers[0], sample_writers[0], diagnostic_csv_writers[0]);
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
            sample_writers[0], diagnostic_csv_writers[0]);
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
            init_writers[0], sample_writers[0], diagnostic_csv_writers[0]);
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
            sample_writers[0], diagnostic_csv_writers[0]);
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
            logger, init_writers[0], sample_writers[0],
            diagnostic_csv_writers[0]);
      }
    }
    // ---- sample end ---- //
  } else if (user_method->arg("variational")) {
    // ---- variational start ---- //
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
          logger, init_writers[0], sample_writers[0],
          diagnostic_csv_writers[0]);
    } else if (algo->value() == "meanfield") {
      return_code = stan::services::experimental::advi::meanfield(
          model, *(init_contexts[0]), random_seed, id, init_radius,
          grad_samples, elbo_samples, max_iterations, tol_rel_obj, eta,
          adapt_engaged, adapt_iterations, eval_elbo, output_samples, interrupt,
          logger, init_writers[0], sample_writers[0],
          diagnostic_csv_writers[0]);
    }
    // ---- variational end ---- //
  }
  //////////////////////////////////////////////////

  stan::math::profile_map &profile_data = get_stan_profile_data();
  if (profile_data.size() > 0) {
    std::string profile_file_name
        = dynamic_cast<string_argument *>(
              parser.arg("output")->arg("profile_file"))
              ->value();
    std::fstream profile_stream(profile_file_name.c_str(), std::fstream::out);
    if (sig_figs > -1) {
      profile_stream << std::setprecision(sig_figs);
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
