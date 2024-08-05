#ifndef CMDSTAN_COMMAND_HPP
#define CMDSTAN_COMMAND_HPP

#include <cmdstan/arguments/arg_data.hpp>
#include <cmdstan/arguments/arg_id.hpp>
#include <cmdstan/arguments/arg_init.hpp>
#include <cmdstan/arguments/arg_output.hpp>
#include <cmdstan/arguments/arg_num_threads.hpp>
#include <cmdstan/arguments/arg_random.hpp>
#include <cmdstan/arguments/arg_opencl.hpp>
#include <cmdstan/arguments/arg_profile_file.hpp>
#include <cmdstan/arguments/argument_parser.hpp>
#include <cmdstan/command_helper.hpp>
#include <cmdstan/return_codes.hpp>
#include <cmdstan/write_model.hpp>
#include <cmdstan/write_stan.hpp>
#include <cmdstan/write_config.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/math/prim/core/init_threadpool_tbb.hpp>
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
#include <tuple>
#include <vector>

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
  unsigned int id = get_arg_val<int_argument>(parser, "id");
  unsigned int num_chains = get_num_chains(parser, id);
  check_file_config(parser);

  parser.print(info);
  write_parallel_info(info);
  write_opencl_device(info);
  info();

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

  std::stringstream msg;

  //////////////////////////////////////////////////
  //           Configure callback writers         //
  //////////////////////////////////////////////////
  auto user_method = parser.arg("method");
  int sig_figs = get_arg_val<int_argument>(parser, "output", "sig_figs");
  int refresh = get_arg_val<int_argument>(parser, "output", "refresh");
  std::string output_file
      = get_arg_val<string_argument>(parser, "output", "file");
  std::string diagnostic_file
      = get_arg_val<string_argument>(parser, "output", "diagnostic_file");

  stan::callbacks::interrupt interrupt;
  stan::callbacks::json_writer<std::ofstream> dummy_json_writer;  // pathfinder
  stan::callbacks::writer init_writer;  // unused - save param initializations
  std::vector<stan::callbacks::writer> init_writers{num_chains,
                                                    stan::callbacks::writer{}};
  std::vector<stan::callbacks::unique_stream_writer<std::ofstream>>
      sample_writers;
  std::vector<stan::callbacks::unique_stream_writer<std::ofstream>>
      diagnostic_csv_writers;
  std::vector<stan::callbacks::json_writer<std::ofstream>>
      diagnostic_json_writers;
  std::vector<stan::callbacks::json_writer<std::ofstream>> metric_json_writers;

  bool save_single_paths
      = user_method->arg("pathfinder")
        && (get_arg_val<bool_argument>(parser, "method", "pathfinder",
                                       "save_single_paths"));

  if (user_method->arg("pathfinder")) {
    if (save_single_paths && diagnostic_file.empty()) {
      diagnostic_file = output_file;
    }
    if (num_chains == 1) {
      init_filestream_writers(sample_writers, num_chains, id, output_file, "",
                              ".csv", sig_figs, "# ");
      if (!diagnostic_file.empty()) {
        save_single_paths = true;
        init_filestream_writers(diagnostic_json_writers, num_chains, id,
                                diagnostic_file, "", ".json", sig_figs);
      } else {
        init_null_writers(diagnostic_json_writers, num_chains);
      }
    } else {
      if (save_single_paths || !diagnostic_file.empty()) {
        init_filestream_writers(sample_writers, num_chains, id, output_file,
                                "_path", ".csv", sig_figs, "# ");
        init_filestream_writers(diagnostic_json_writers, num_chains, id,
                                diagnostic_file, "_path", ".json", sig_figs);
      } else {
        init_null_writers(sample_writers, num_chains);
        init_null_writers(diagnostic_json_writers, num_chains);
      }
    }
    init_null_writers(diagnostic_csv_writers, num_chains);
  } else {
    init_filestream_writers(sample_writers, num_chains, id, output_file, "",
                            ".csv", sig_figs, "# ");
    if (!diagnostic_file.empty()) {
      if (user_method->arg("laplace")) {
        init_filestream_writers(diagnostic_json_writers, num_chains, id,
                                diagnostic_file, "", ".json", sig_figs);
        init_null_writers(diagnostic_csv_writers, num_chains);

      } else {
        init_filestream_writers(diagnostic_csv_writers, num_chains, id,
                                diagnostic_file, "", ".csv", sig_figs, "# ");
        init_null_writers(diagnostic_json_writers, num_chains);
      }
    } else {
      init_null_writers(diagnostic_csv_writers, num_chains);
      init_null_writers(diagnostic_json_writers, num_chains);
    }
  }
  if (user_method->arg("sample")
      && get_arg_val<bool_argument>(parser, "method", "sample", "adapt",
                                    "save_metric")) {
    // adaptation must be engaged
    if (!get_arg_val<bool_argument>(parser, "method", "sample", "adapt",
                                    "engaged")) {
      msg << "Saving the adapted metric (save_metric=1) can only be enabled "
             "when adaptation is engaged (adapt engaged=1)"
          << std::endl;
      throw std::invalid_argument(msg.str());
    }
    init_filestream_writers(metric_json_writers, num_chains, id, output_file,
                            "_metric", ".json", sig_figs);
  } else {
    init_null_writers(metric_json_writers, num_chains);
  }

  // Setup initial parameter values - arg "init"
  // arg is either filename or init radius value
  std::string init = get_arg_val<string_argument>(parser, "init");
  double init_radius = 2.0;
  try {
    init_radius = std::stod(init);
    init = "";
  } catch (const std::logic_error &e) {
  }

  std::vector<std::shared_ptr<stan::io::var_context>> init_contexts
      = get_vec_var_context(init, num_chains, id);

  if (get_arg_val<bool_argument>(parser, "output", "save_cmdstan_config")) {
    auto config_filename
        = file::get_basename_suffix(output_file).first + "_config.json";
    auto ofs_args = file::safe_create(config_filename, sig_figs);
    stan::callbacks::json_writer<std::ostream> json_args(std::move(ofs_args));
    write_config(json_args, parser, model);
  }

  for (int i = 0; i < num_chains; ++i) {
    write_config(sample_writers[i], parser, model);
    write_stan(diagnostic_csv_writers[i]);
    write_model(diagnostic_csv_writers[i], model.model_name());
    parser.print(diagnostic_csv_writers[i]);
  }

  //////////////////////////////////////////////////
  //            Invoke Services                   //
  //////////////////////////////////////////////////
  int return_code = return_codes::NOT_OK;

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
    bool psis_resample
        = get_arg_val<bool_argument>(*pathfinder_arg, "psis_resample");
    bool calculate_lp
        = get_arg_val<bool_argument>(*pathfinder_arg, "calculate_lp");
    if (num_psis_draws > num_draws * num_chains) {
      logger.warn(
          "Warning: Number of PSIS draws is larger than the total number of "
          "draws returned by the single Pathfinders. This is likely "
          "unintentional and leads to re-sampling from the same draws.");
    }
    if (model.num_params_r() == 0) {
      throw std::invalid_argument(
          "Model has 0 parameters, cannot run Pathfinder.");
    }

    if (num_chains == 1) {
      return_code = stan::services::pathfinder::pathfinder_lbfgs_single<
          false, stan::model::model_base>(
          model, *(init_contexts[0]), random_seed, id, init_radius,
          history_size, init_alpha, tol_obj, tol_rel_obj, tol_grad,
          tol_rel_grad, tol_param, max_lbfgs_iters, num_elbo_draws, num_draws,
          save_single_paths, refresh, interrupt, logger, init_writer,
          sample_writers[0], diagnostic_json_writers[0], calculate_lp);
    } else {
      auto output_filenames
          = file::make_filenames(output_file, "", ".csv", 1, id);
      auto ofs = file::safe_create(output_filenames[0], sig_figs);
      stan::callbacks::unique_stream_writer<std::ofstream> pathfinder_writer(
          std::move(ofs), "# ");
      write_config(pathfinder_writer, parser, model);
      return_code = stan::services::pathfinder::pathfinder_lbfgs_multi<
          stan::model::model_base>(
          model, init_contexts, random_seed, id, init_radius, history_size,
          init_alpha, tol_obj, tol_rel_obj, tol_grad, tol_rel_grad, tol_param,
          max_lbfgs_iters, num_elbo_draws, num_draws, num_psis_draws,
          num_chains, save_single_paths, refresh, interrupt, logger,
          init_writers, sample_writers, diagnostic_json_writers,
          pathfinder_writer, dummy_json_writer, calculate_lp, psis_resample);
    }
    // ---- pathfinder end ---- //
  } else if (user_method->arg("generate_quantities")) {
    // ---- generate_quantities start ---- //
    auto gq_arg = parser.arg("method")->arg("generate_quantities");
    std::string fname = get_arg_val<string_argument>(*gq_arg, "fitted_params");
    if (fname.empty()) {
      throw std::invalid_argument(
          "Missing fitted_params argument, cannot run generate_quantities "
          "without fitted sample.");
    }
    auto file_info = file::get_basename_suffix(fname);
    if (file_info.second != ".csv") {
      throw std::invalid_argument("Fitted params file must be a CSV file.");
    }
    std::vector<std::string> fname_vec
        = file::make_filenames(file_info.first, "", ".csv", num_chains, id);
    std::vector<std::string> param_names = get_constrained_param_names(model);
    std::vector<Eigen::MatrixXd> fitted_params_vec;
    fitted_params_vec.reserve(num_chains);
    for (int i = 0; i < num_chains; ++i) {
      stan::io::stan_csv fitted_params;
      size_t col_offset, num_rows, num_cols;
      parse_stan_csv(fname_vec[i], model, param_names, fitted_params,
                     col_offset, num_rows, num_cols);
      fitted_params_vec.emplace_back(
          fitted_params.samples.block(0, col_offset, num_rows, num_cols));
    }
    return_code = stan::services::standalone_generate(
        model, num_chains, fitted_params_vec, random_seed, interrupt, logger,
        sample_writers);
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
    bool calculate_lp
        = get_arg_val<bool_argument>(*laplace_arg, "calculate_lp");
    int draws = get_arg_val<int_argument>(*laplace_arg, "draws");
    if (jacobian) {
      return_code = stan::services::laplace_sample<true>(
          model, theta_hat, draws, calculate_lp, random_seed, refresh,
          interrupt, logger, sample_writers[0], diagnostic_json_writers[0]);
    } else {
      return_code = stan::services::laplace_sample<false>(
          model, theta_hat, draws, calculate_lp, random_seed, refresh,
          interrupt, logger, sample_writers[0], diagnostic_json_writers[0]);
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
      if (file::get_suffix(cpars_file) == ".csv") {
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
      msg << "Error during log_prob calculation:" << std::endl;
      msg << e.what() << std::endl;
      logger.error(msg.str());
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
    int num_warmup
        = get_arg_val<int_argument>(parser, "method", "sample", "num_warmup");
    int num_samples
        = get_arg_val<int_argument>(parser, "method", "sample", "num_samples");
    int num_thin
        = get_arg_val<int_argument>(parser, "method", "sample", "thin");
    bool save_warmup
        = get_arg_val<bool_argument>(parser, "method", "sample", "save_warmup");

    list_argument *algo = dynamic_cast<list_argument *>(
        parser.arg("method")->arg("sample")->arg("algorithm"));
    std::string algo_name = algo->value();

    bool adapt_engaged = get_arg_val<bool_argument>(parser, "method", "sample",
                                                    "adapt", "engaged");
    if (algo_name != "fixed_param" && adapt_engaged == true
        && num_warmup == 0) {
      msg << "The number of warmup samples (num_warmup) must be greater than "
          << "zero if adaptation is enabled." << std::endl;
      throw std::invalid_argument(msg.str());
    }

    if (algo_name == "fixed_param") {
      return_code = stan::services::sample::fixed_param(
          model, num_chains, init_contexts, random_seed, id, init_radius,
          num_samples, num_thin, refresh, interrupt, logger, init_writers,
          sample_writers, diagnostic_csv_writers);
    } else if (algo_name == "hmc") {
      list_argument *metric_arg
          = dynamic_cast<list_argument *>(parser.arg("method")
                                              ->arg("sample")
                                              ->arg("algorithm")
                                              ->arg("hmc")
                                              ->arg("metric"));
      std::string metric = metric_arg->value();
      std::string metric_file = get_arg_val<string_argument>(
          parser, "method", "sample", "algorithm", "hmc", "metric_file");
      bool metric_supplied = !metric_file.empty();
      context_vector metric_contexts;
      if (metric_supplied) {
        metric_contexts = get_vec_var_context(metric_file, num_chains, id);
      }
      double stepsize = get_arg_val<real_argument>(
          parser, "method", "sample", "algorithm", "hmc", "stepsize");
      double jitter = get_arg_val<real_argument>(
          parser, "method", "sample", "algorithm", "hmc", "stepsize_jitter");
      list_argument *hmc_engine
          = dynamic_cast<list_argument *>(algo->arg("hmc")->arg("engine"));
      std::string engine = hmc_engine->value();
      if (engine == "nuts") {
        int max_depth
            = get_arg_val<int_argument>(parser, "method", "sample", "algorithm",
                                        "hmc", "engine", "nuts", "max_depth");
        if (adapt_engaged == false) {
          // NUTS, no adaptation
          if (metric == "dense_e" && metric_supplied == true) {
            return_code = stan::services::sample::hmc_nuts_dense_e(
                model, num_chains, init_contexts, metric_contexts, random_seed,
                id, init_radius, num_warmup, num_samples, num_thin, save_warmup,
                refresh, stepsize, jitter, max_depth, interrupt, logger,
                init_writers, sample_writers, diagnostic_csv_writers);
          } else if (metric == "dense_e") {
            return_code = stan::services::sample::hmc_nuts_dense_e(
                model, num_chains, init_contexts, random_seed, id, init_radius,
                num_warmup, num_samples, num_thin, save_warmup, refresh,
                stepsize, jitter, max_depth, interrupt, logger, init_writers,
                sample_writers, diagnostic_csv_writers);
          } else if (metric == "diag_e" && metric_supplied == true) {
            return_code = stan::services::sample::hmc_nuts_diag_e(
                model, num_chains, init_contexts, metric_contexts, random_seed,
                id, init_radius, num_warmup, num_samples, num_thin, save_warmup,
                refresh, stepsize, jitter, max_depth, interrupt, logger,
                init_writers, sample_writers, diagnostic_csv_writers);
          } else if (metric == "diag_e") {
            return_code = stan::services::sample::hmc_nuts_diag_e(
                model, num_chains, init_contexts, random_seed, id, init_radius,
                num_warmup, num_samples, num_thin, save_warmup, refresh,
                stepsize, jitter, max_depth, interrupt, logger, init_writers,
                sample_writers, diagnostic_csv_writers);
          } else if (metric == "unit_e") {
            return_code = stan::services::sample::hmc_nuts_unit_e(
                model, num_chains, init_contexts, random_seed, id, init_radius,
                num_warmup, num_samples, num_thin, save_warmup, refresh,
                stepsize, jitter, max_depth, interrupt, logger, init_writers,
                sample_writers, diagnostic_csv_writers);
          }
        } else {
          // NUTS adaptation
          double delta = get_arg_val<real_argument>(parser, "method", "sample",
                                                    "adapt", "delta");
          double gamma = get_arg_val<real_argument>(parser, "method", "sample",
                                                    "adapt", "gamma");
          double kappa = get_arg_val<real_argument>(parser, "method", "sample",
                                                    "adapt", "kappa");
          double t0 = get_arg_val<real_argument>(parser, "method", "sample",
                                                 "adapt", "t0");
          unsigned int init_buffer = get_arg_val<u_int_argument>(
              parser, "method", "sample", "adapt", "init_buffer");
          unsigned int term_buffer = get_arg_val<u_int_argument>(
              parser, "method", "sample", "adapt", "term_buffer");
          unsigned int window = get_arg_val<u_int_argument>(
              parser, "method", "sample", "adapt", "window");

          if (metric == "dense_e" && metric_supplied == true) {
            return_code = stan::services::sample::hmc_nuts_dense_e_adapt(
                model, num_chains, init_contexts, metric_contexts, random_seed,
                id, init_radius, num_warmup, num_samples, num_thin, save_warmup,
                refresh, stepsize, jitter, max_depth, delta, gamma, kappa, t0,
                init_buffer, term_buffer, window, interrupt, logger,
                init_writers, sample_writers, diagnostic_csv_writers,
                metric_json_writers);
          } else if (metric == "dense_e") {
            return_code = stan::services::sample::hmc_nuts_dense_e_adapt(
                model, num_chains, init_contexts, random_seed, id, init_radius,
                num_warmup, num_samples, num_thin, save_warmup, refresh,
                stepsize, jitter, max_depth, delta, gamma, kappa, t0,
                init_buffer, term_buffer, window, interrupt, logger,
                init_writers, sample_writers, diagnostic_csv_writers,
                metric_json_writers);
          } else if (metric == "diag_e" && metric_supplied == true) {
            return_code = stan::services::sample::hmc_nuts_diag_e_adapt(
                model, num_chains, init_contexts, metric_contexts, random_seed,
                id, init_radius, num_warmup, num_samples, num_thin, save_warmup,
                refresh, stepsize, jitter, max_depth, delta, gamma, kappa, t0,
                init_buffer, term_buffer, window, interrupt, logger,
                init_writers, sample_writers, diagnostic_csv_writers,
                metric_json_writers);
          } else if (metric == "diag_e") {
            return_code = stan::services::sample::hmc_nuts_diag_e_adapt(
                model, num_chains, init_contexts, random_seed, id, init_radius,
                num_warmup, num_samples, num_thin, save_warmup, refresh,
                stepsize, jitter, max_depth, delta, gamma, kappa, t0,
                init_buffer, term_buffer, window, interrupt, logger,
                init_writers, sample_writers, diagnostic_csv_writers,
                metric_json_writers);
          } else if (metric == "unit_e") {
            return_code = stan::services::sample::hmc_nuts_unit_e_adapt(
                model, num_chains, init_contexts, random_seed, id, init_radius,
                num_warmup, num_samples, num_thin, save_warmup, refresh,
                stepsize, jitter, max_depth, delta, gamma, kappa, t0, interrupt,
                logger, init_writers, sample_writers, diagnostic_csv_writers,
                metric_json_writers);
          }
        }
      } else if (engine == "static") {
        double int_time = get_arg_val<real_argument>(
            parser, "method", "sample", "algorithm", "hmc", "engine", "static",
            "int_time");
        if (adapt_engaged == false) {  // static, no adaptation
          if (metric == "dense_e" && metric_supplied == true) {
            return_code = stan::services::sample::hmc_static_dense_e(
                model, *(init_contexts[0]), *(metric_contexts[0]), random_seed,
                id, init_radius, num_warmup, num_samples, num_thin, save_warmup,
                refresh, stepsize, jitter, int_time, interrupt, logger,
                init_writers[0], sample_writers[0], diagnostic_csv_writers[0]);
          } else if (metric == "dense_e") {
            return_code = stan::services::sample::hmc_static_dense_e(
                model, *(init_contexts[0]), random_seed, id, init_radius,
                num_warmup, num_samples, num_thin, save_warmup, refresh,
                stepsize, jitter, int_time, interrupt, logger, init_writers[0],
                sample_writers[0], diagnostic_csv_writers[0]);
          } else if (metric == "diag_e" && metric_supplied == true) {
            return_code = stan::services::sample::hmc_static_diag_e(
                model, *(init_contexts[0]), *(metric_contexts[0]), random_seed,
                id, init_radius, num_warmup, num_samples, num_thin, save_warmup,
                refresh, stepsize, jitter, int_time, interrupt, logger,
                init_writers[0], sample_writers[0], diagnostic_csv_writers[0]);
          } else if (metric == "diag_e") {
            return_code = stan::services::sample::hmc_static_diag_e(
                model, *(init_contexts[0]), random_seed, id, init_radius,
                num_warmup, num_samples, num_thin, save_warmup, refresh,
                stepsize, jitter, int_time, interrupt, logger, init_writers[0],
                sample_writers[0], diagnostic_csv_writers[0]);
          } else if (metric == "unit_e") {
            return_code = stan::services::sample::hmc_static_unit_e(
                model, *(init_contexts[0]), random_seed, id, init_radius,
                num_warmup, num_samples, num_thin, save_warmup, refresh,
                stepsize, jitter, int_time, interrupt, logger, init_writers[0],
                sample_writers[0], diagnostic_csv_writers[0]);
          }
        } else {  // static adaptation
          double delta = get_arg_val<real_argument>(parser, "method", "sample",
                                                    "adapt", "delta");
          double gamma = get_arg_val<real_argument>(parser, "method", "sample",
                                                    "adapt", "gamma");
          double kappa = get_arg_val<real_argument>(parser, "method", "sample",
                                                    "adapt", "kappa");
          double t0 = get_arg_val<real_argument>(parser, "method", "sample",
                                                 "adapt", "t0");
          unsigned int init_buffer = get_arg_val<u_int_argument>(
              parser, "method", "sample", "adapt", "init_buffer");
          unsigned int term_buffer = get_arg_val<u_int_argument>(
              parser, "method", "sample", "adapt", "term_buffer");
          unsigned int window = get_arg_val<u_int_argument>(
              parser, "method", "sample", "adapt", "window");
          if (metric == "dense_e" && metric_supplied == true) {
            return_code = stan::services::sample::hmc_static_dense_e_adapt(
                model, *(init_contexts[0]), *(metric_contexts[0]), random_seed,
                id, init_radius, num_warmup, num_samples, num_thin, save_warmup,
                refresh, stepsize, jitter, int_time, delta, gamma, kappa, t0,
                init_buffer, term_buffer, window, interrupt, logger,
                init_writers[0], sample_writers[0], diagnostic_csv_writers[0]);
          } else if (metric == "dense_e") {
            return_code = stan::services::sample::hmc_static_dense_e_adapt(
                model, *(init_contexts[0]), random_seed, id, init_radius,
                num_warmup, num_samples, num_thin, save_warmup, refresh,
                stepsize, jitter, int_time, delta, gamma, kappa, t0,
                init_buffer, term_buffer, window, interrupt, logger,
                init_writers[0], sample_writers[0], diagnostic_csv_writers[0]);
          } else if (metric == "diag_e" && metric_supplied == true) {
            return_code = stan::services::sample::hmc_static_diag_e_adapt(
                model, *(init_contexts[0]), *(metric_contexts[0]), random_seed,
                id, init_radius, num_warmup, num_samples, num_thin, save_warmup,
                refresh, stepsize, jitter, int_time, delta, gamma, kappa, t0,
                init_buffer, term_buffer, window, interrupt, logger,
                init_writers[0], sample_writers[0], diagnostic_csv_writers[0]);
          } else if (metric == "diag_e") {
            return_code = stan::services::sample::hmc_static_diag_e_adapt(
                model, *(init_contexts[0]), random_seed, id, init_radius,
                num_warmup, num_samples, num_thin, save_warmup, refresh,
                stepsize, jitter, int_time, delta, gamma, kappa, t0,
                init_buffer, term_buffer, window, interrupt, logger,
                init_writers[0], sample_writers[0], diagnostic_csv_writers[0]);
          } else if (metric == "unit_e") {
            return_code = stan::services::sample::hmc_static_unit_e_adapt(
                model, *(init_contexts[0]), random_seed, id, init_radius,
                num_warmup, num_samples, num_thin, save_warmup, refresh,
                stepsize, jitter, int_time, delta, gamma, kappa, t0, interrupt,
                logger, init_writers[0], sample_writers[0],
                diagnostic_csv_writers[0]);
          }
        }
      }  // end static HMC
    }    // ---- sample end ---- //
  } else if (user_method->arg("variational")) {
    // ---- variational start ---- //
    list_argument *algo = dynamic_cast<list_argument *>(
        parser.arg("method")->arg("variational")->arg("algorithm"));
    std::string algorithm = algo->value();
    int grad_samples = get_arg_val<int_argument>(parser, "method",
                                                 "variational", "grad_samples");
    int elbo_samples = get_arg_val<int_argument>(parser, "method",
                                                 "variational", "elbo_samples");
    int max_iterations
        = get_arg_val<int_argument>(parser, "method", "variational", "iter");
    double tol_rel_obj = get_arg_val<real_argument>(
        parser, "method", "variational", "tol_rel_obj");
    double eta
        = get_arg_val<real_argument>(parser, "method", "variational", "eta");
    bool adapt_engaged = get_arg_val<bool_argument>(
        parser, "method", "variational", "adapt", "engaged");
    int adapt_iterations = get_arg_val<int_argument>(
        parser, "method", "variational", "adapt", "iter");
    int eval_elbo = get_arg_val<int_argument>(parser, "method", "variational",
                                              "eval_elbo");
    int output_samples = get_arg_val<int_argument>(
        parser, "method", "variational", "output_samples");
    if (algorithm == "fullrank") {
      return_code = stan::services::experimental::advi::fullrank(
          model, *(init_contexts[0]), random_seed, id, init_radius,
          grad_samples, elbo_samples, max_iterations, tol_rel_obj, eta,
          adapt_engaged, adapt_iterations, eval_elbo, output_samples, interrupt,
          logger, init_writers[0], sample_writers[0],
          diagnostic_csv_writers[0]);
    } else if (algorithm == "meanfield") {
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
        = get_arg_val<string_argument>(parser, "output", "profile_file");
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
