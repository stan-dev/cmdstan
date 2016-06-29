#ifndef CMDSTAN_COMMAND_HPP
#define CMDSTAN_COMMAND_HPP

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/random/additive_combine.hpp>  // L'Ecuyer RNG
#include <boost/random/uniform_real_distribution.hpp>

#include <stan/version.hpp>
#include <stan/io/cmd_line.hpp>
#include <stan/io/dump.hpp>
#include <stan/io/json/json_data.hpp>
#include <stan/io/json/json_data_handler.hpp>
#include <stan/io/json/json_error.hpp>
#include <stan/io/json/json_handler.hpp>
#include <stan/io/json/json_parser.hpp>

#include <stan/old_services/arguments/arg_adapt.hpp>
#include <stan/old_services/arguments/arg_adapt_delta.hpp>
#include <stan/old_services/arguments/arg_adapt_engaged.hpp>
#include <stan/old_services/arguments/arg_adapt_gamma.hpp>
#include <stan/old_services/arguments/arg_adapt_init_buffer.hpp>
#include <stan/old_services/arguments/arg_adapt_kappa.hpp>
#include <stan/old_services/arguments/arg_adapt_t0.hpp>
#include <stan/old_services/arguments/arg_adapt_term_buffer.hpp>
#include <stan/old_services/arguments/arg_adapt_window.hpp>
#include <stan/old_services/arguments/arg_bfgs.hpp>
#include <stan/old_services/arguments/arg_data.hpp>
#include <stan/old_services/arguments/arg_data_file.hpp>
#include <stan/old_services/arguments/arg_dense_e.hpp>
#include <stan/old_services/arguments/arg_diag_e.hpp>
#include <stan/old_services/arguments/arg_diagnose.hpp>
#include <stan/old_services/arguments/arg_diagnostic_file.hpp>
#include <stan/old_services/arguments/arg_engine.hpp>
#include <stan/old_services/arguments/arg_fail.hpp>
#include <stan/old_services/arguments/arg_fixed_param.hpp>
#include <stan/old_services/arguments/arg_history_size.hpp>
#include <stan/old_services/arguments/arg_hmc.hpp>
#include <stan/old_services/arguments/arg_id.hpp>
#include <stan/old_services/arguments/arg_init.hpp>
#include <stan/old_services/arguments/arg_init_alpha.hpp>
#include <stan/old_services/arguments/arg_int_time.hpp>
#include <stan/old_services/arguments/arg_iter.hpp>
#include <stan/old_services/arguments/arg_lbfgs.hpp>
#include <stan/old_services/arguments/arg_max_depth.hpp>
#include <stan/old_services/arguments/arg_method.hpp>
#include <stan/old_services/arguments/arg_metric.hpp>
#include <stan/old_services/arguments/arg_newton.hpp>
#include <stan/old_services/arguments/arg_num_samples.hpp>
#include <stan/old_services/arguments/arg_num_warmup.hpp>
#include <stan/old_services/arguments/arg_nuts.hpp>
#include <stan/old_services/arguments/arg_optimize.hpp>
#include <stan/old_services/arguments/arg_optimize_algo.hpp>
#include <stan/old_services/arguments/arg_output.hpp>
#include <stan/old_services/arguments/arg_output_file.hpp>
#include <stan/old_services/arguments/arg_random.hpp>
#include <stan/old_services/arguments/arg_refresh.hpp>
#include <stan/old_services/arguments/arg_rwm.hpp>
#include <stan/old_services/arguments/arg_sample.hpp>
#include <stan/old_services/arguments/arg_sample_algo.hpp>
#include <stan/old_services/arguments/arg_save_iterations.hpp>
#include <stan/old_services/arguments/arg_save_warmup.hpp>
#include <stan/old_services/arguments/arg_seed.hpp>
#include <stan/old_services/arguments/arg_static.hpp>
#include <stan/old_services/arguments/arg_stepsize.hpp>
#include <stan/old_services/arguments/arg_stepsize_jitter.hpp>
#include <stan/old_services/arguments/arg_test.hpp>
#include <stan/old_services/arguments/arg_test_grad_eps.hpp>
#include <stan/old_services/arguments/arg_test_grad_err.hpp>
#include <stan/old_services/arguments/arg_test_gradient.hpp>
#include <stan/old_services/arguments/arg_thin.hpp>
#include <stan/old_services/arguments/arg_tolerance.hpp>
#include <stan/old_services/arguments/arg_unit_e.hpp>
#include <stan/old_services/arguments/argument.hpp>
#include <stan/old_services/arguments/argument_parser.hpp>
#include <stan/old_services/arguments/argument_probe.hpp>
#include <stan/old_services/arguments/categorical_argument.hpp>
#include <stan/old_services/arguments/list_argument.hpp>
#include <stan/old_services/arguments/singleton_argument.hpp>
#include <stan/old_services/arguments/unvalued_argument.hpp>
#include <stan/old_services/arguments/valued_argument.hpp>
#include <stan/old_services/sample/mcmc_writer.hpp>
#include <stan/mcmc/fixed_param_sampler.hpp>
#include <stan/mcmc/hmc/static/adapt_unit_e_static_hmc.hpp>
#include <stan/mcmc/hmc/static/adapt_diag_e_static_hmc.hpp>
#include <stan/mcmc/hmc/static/adapt_dense_e_static_hmc.hpp>
#include <stan/mcmc/hmc/nuts/adapt_unit_e_nuts.hpp>
#include <stan/mcmc/hmc/nuts/adapt_diag_e_nuts.hpp>
#include <stan/mcmc/hmc/nuts/adapt_dense_e_nuts.hpp>

#include <stan/model/util.hpp>

#include <stan/optimization/newton.hpp>
#include <stan/optimization/bfgs.hpp>

#include <stan/variational/advi.hpp>

#include <stan/old_services/init/initialize_state.hpp>
#include <stan/old_services/io/do_print.hpp>
#include <stan/old_services/io/write_error_msg.hpp>
#include <stan/old_services/io/write_iteration.hpp>
#include <stan/old_services/io/write_model.hpp>
#include <stan/old_services/io/write_stan.hpp>
#include <stan/old_services/mcmc/sample.hpp>
#include <stan/old_services/mcmc/warmup.hpp>
#include <stan/old_services/optimize/do_bfgs_optimize.hpp>
#include <stan/old_services/sample/init_adapt.hpp>
#include <stan/old_services/sample/init_nuts.hpp>
#include <stan/old_services/sample/init_static_hmc.hpp>
#include <stan/old_services/sample/init_windowed_adapt.hpp>
#include <stan/old_services/sample/progress.hpp>

#include <stan/interface_callbacks/interrupt/noop.hpp>
#include <stan/interface_callbacks/var_context_factory/dump_factory.hpp>
#include <stan/interface_callbacks/writer/base_writer.hpp>
#include <stan/interface_callbacks/writer/stream_writer.hpp>

#include <stan/services/diagnose/diagnose.hpp>
#include <stan/services/optimize/bfgs.hpp>
#include <stan/services/optimize/lbfgs.hpp>
#include <stan/services/optimize/newton.hpp>
#include <stan/services/sample/fixed_param.hpp>
#include <stan/services/sample/hmc_nuts_dense_e.hpp>
#include <stan/services/sample/hmc_nuts_dense_e_adapt.hpp>
#include <stan/io/empty_var_context.hpp>

#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace stan {
  namespace services {

    class null_fstream : public std::fstream {
    public:
      null_fstream() {}
    };

    template <class Model>
    int command(int argc, const char* argv[]) {
      interface_callbacks::interrupt::noop interrupt;
      
      stan::interface_callbacks::writer::stream_writer info(std::cout);
      stan::interface_callbacks::writer::stream_writer err(std::cout);
      
      std::vector<stan::services::argument*> valid_arguments;
      valid_arguments.push_back(new stan::services::arg_id());
      valid_arguments.push_back(new stan::services::arg_data());
      valid_arguments.push_back(new stan::services::arg_init());
      valid_arguments.push_back(new stan::services::arg_random());
      valid_arguments.push_back(new stan::services::arg_output());

      stan::services::argument_parser parser(valid_arguments);
      int err_code = parser.parse_args(argc, argv, info, err);
      if (err_code != 0) {
        std::cout << "Failed to parse arguments, terminating Stan" << std::endl;
        return err_code;
      }

      if (parser.help_printed())
        return err_code;

      //////////////////////////////////////////////////
      //                  Input/Output                //
      //////////////////////////////////////////////////

      // Data input
      std::string data_file
        = dynamic_cast<stan::services::string_argument*>
        (parser.arg("data")->arg("file"))->value();

      std::fstream data_stream(data_file.c_str(),
                               std::fstream::in);
      stan::io::dump data_var_context(data_stream);
      data_stream.close();
      
      // Sample output
      std::string output_file = dynamic_cast<stan::services::string_argument*>(
                                parser.arg("output")->arg("file"))->value();
      std::fstream* output_stream = 0;
      if (output_file != "") {
        output_stream = new std::fstream(output_file.c_str(),
                                         std::fstream::out);
      } else {
        output_stream = new null_fstream();
      }

      
      // Diagnostic output
      std::string diagnostic_file
        = dynamic_cast<stan::services::string_argument*>
          (parser.arg("output")->arg("diagnostic_file"))->value();

      std::fstream* diagnostic_stream = 0;
      if (diagnostic_file != "") {
        diagnostic_stream = new std::fstream(diagnostic_file.c_str(),
                                             std::fstream::out);
      } else {
        diagnostic_stream = new null_fstream();
      }

      // Refresh rate
      int refresh = dynamic_cast<stan::services::int_argument*>(
                    parser.arg("output")->arg("refresh"))->value();
      
      // Identification
      unsigned int id = dynamic_cast<stan::services::int_argument*>
        (parser.arg("id"))->value();

      stan::interface_callbacks::writer::stream_writer
        sample_writer(*output_stream, "# "),
        diagnostic_writer(*diagnostic_stream, "# ");

      //////////////////////////////////////////////////
      //            Random number generator           //
      //////////////////////////////////////////////////

      unsigned int random_seed = 0;

      stan::services::u_int_argument* random_arg
        = dynamic_cast<stan::services::u_int_argument*>
        (parser.arg("random")->arg("seed"));

      if (random_arg->is_default()) {
        random_seed
          = (boost::posix_time::microsec_clock::universal_time() -
             boost::posix_time::ptime(boost::posix_time::min_date_time))
          .total_milliseconds();

        random_arg->set_value(random_seed);

      } else {
        random_seed = random_arg->value();
      }

      // FIXME: remove after service methods in
      typedef boost::ecuyer1988 rng_t;  // (2**50 = 1T samples, 1000 chains)
      rng_t base_rng(random_seed);

      // Advance generator to avoid process conflicts
      static boost::uintmax_t DISCARD_STRIDE
        = static_cast<boost::uintmax_t>(1) << 50;
      base_rng.discard(DISCARD_STRIDE * (id - 1));
      // FIXME: remove after service methods in

      

      //////////////////////////////////////////////////
      //                Initialize Model              //
      //////////////////////////////////////////////////

      Model model(data_var_context, &std::cout);

      Eigen::VectorXd cont_params = Eigen::VectorXd::Zero(model.num_params_r());

      parser.print(info);
      info();
      
      if (output_stream) {
        io::write_stan(sample_writer);
        io::write_model(sample_writer, model.model_name());
        parser.print(sample_writer);
      }

      if (diagnostic_stream) {
        io::write_stan(diagnostic_writer);
        io::write_model(diagnostic_writer, model.model_name());
        parser.print(diagnostic_writer);
      }
      
      std::string init = dynamic_cast<stan::services::string_argument*>(
                         parser.arg("init"))->value();

      // FIXME: remove after service methods in
      interface_callbacks::var_context_factory::dump_factory var_context_factory;
      if (!init::initialize_state<interface_callbacks::var_context_factory::dump_factory>
          (init, cont_params, model, base_rng, info,
           var_context_factory))
        return stan::services::error_codes::SOFTWARE;
      // FIXME:: remove

      double init_radius = 2.0;
      try {
        init_radius = boost::lexical_cast<double>(init);
      } catch (const boost::bad_lexical_cast& e) {
      }
      std::fstream* init_stream = 0;
      if (init != "") {
        init_stream = new std::fstream(init.c_str(),
                                       std::fstream::in);
      } else {
        init_stream = new null_fstream();
      }
      stan::io::dump init_context(*init_stream);
      delete init_stream;
      
      //////////////////////////////////////////////////
      //               Model Diagnostics              //
      //////////////////////////////////////////////////

      if (parser.arg("method")->arg("diagnose")) {
        stan::services::list_argument* test = dynamic_cast<stan::services::list_argument*>
          (parser.arg("method")->arg("diagnose")->arg("test"));
        
        if (test->value() == "gradient") {
          double epsilon = dynamic_cast<stan::services::real_argument*>
                           (test->arg("gradient")->arg("epsilon"))->value();

          double error = dynamic_cast<stan::services::real_argument*>
                         (test->arg("gradient")->arg("error"))->value();
          
          return stan::services::diagnose::diagnose(model,
                                                    init_context, 
                                                    random_seed, id,
                                                    init_radius,
                                                    epsilon, error,
                                                    info,
                                                    sample_writer);
        }
        return stan::services::error_codes::OK;
      }

      //////////////////////////////////////////////////
      //           Optimization Algorithms            //
      //////////////////////////////////////////////////

      if (parser.arg("method")->arg("optimize")) {
        stan::services::list_argument* algo = dynamic_cast<stan::services::list_argument*>
                              (parser.arg("method")->arg("optimize")->arg("algorithm"));

        int num_iterations = dynamic_cast<stan::services::int_argument*>(
                             parser.arg("method")->arg("optimize")->arg("iter"))->value();

        bool save_iterations
          = dynamic_cast<stan::services::bool_argument*>(parser.arg("method")
                                         ->arg("optimize")
                                         ->arg("save_iterations"))->value();

        int return_code = stan::services::error_codes::CONFIG;
        if (algo->value() == "newton") {
          return_code = stan::services::optimize::newton(model,
                                                         init_context,
                                                         random_seed,
                                                         id,
                                                         init_radius,
                                                         num_iterations,
                                                         save_iterations,
                                                         interrupt,
                                                         info,
                                                         sample_writer);
        } else if (algo->value() == "bfgs") {
          double init_alpha = dynamic_cast<stan::services::real_argument*>(
                         algo->arg("bfgs")->arg("init_alpha"))->value();
          double tol_obj = dynamic_cast<services::real_argument*>(
                         algo->arg("bfgs")->arg("tol_obj"))->value();
          double tol_rel_obj = dynamic_cast<stan::services::real_argument*>(
                         algo->arg("bfgs")->arg("tol_rel_obj"))->value();
          double tol_grad = dynamic_cast<stan::services::real_argument*>(
                         algo->arg("bfgs")->arg("tol_grad"))->value();
          double tol_rel_grad = dynamic_cast<stan::services::real_argument*>(
                         algo->arg("bfgs")->arg("tol_rel_grad"))->value();
          double tol_param = dynamic_cast<stan::services::real_argument*>(
                         algo->arg("bfgs")->arg("tol_param"))->value();

          return_code = stan::services::optimize::bfgs(model,
                                                       init_context,
                                                       random_seed,
                                                       id,
                                                       init_radius,
                                                       init_alpha,
                                                       tol_obj,
                                                       tol_rel_obj,
                                                       tol_grad,
                                                       tol_rel_grad,
                                                       tol_param,
                                                       num_iterations,
                                                       save_iterations,
                                                       refresh,
                                                       interrupt,
                                                       info,
                                                       sample_writer);
        } else if (algo->value() == "lbfgs") {
          int history_size = dynamic_cast<services::int_argument*>(
                         algo->arg("lbfgs")->arg("history_size"))->value();
          double init_alpha = dynamic_cast<services::real_argument*>(
                         algo->arg("lbfgs")->arg("init_alpha"))->value();
          double tol_obj = dynamic_cast<services::real_argument*>(
                         algo->arg("lbfgs")->arg("tol_obj"))->value();
          double tol_rel_obj = dynamic_cast<services::real_argument*>(
                         algo->arg("lbfgs")->arg("tol_rel_obj"))->value();
          double tol_grad = dynamic_cast<services::real_argument*>(
                         algo->arg("lbfgs")->arg("tol_grad"))->value();
          double tol_rel_grad = dynamic_cast<services::real_argument*>(
                         algo->arg("lbfgs")->arg("tol_rel_grad"))->value();
          double tol_param = dynamic_cast<services::real_argument*>(
                         algo->arg("lbfgs")->arg("tol_param"))->value();

          return_code = stan::services::optimize::lbfgs(model,
                                                        init_context,
                                                        random_seed,
                                                        id,
                                                        init_radius,
                                                        history_size,
                                                        init_alpha,
                                                        tol_obj,
                                                        tol_rel_obj,
                                                        tol_grad,
                                                        tol_rel_grad,
                                                        tol_param,
                                                        num_iterations,
                                                        save_iterations,
                                                        refresh,
                                                        interrupt,
                                                        info,
                                                        sample_writer);
        }
        output_stream->close();
        diagnostic_stream->close();
        delete output_stream;
        delete diagnostic_stream;
        return return_code;
      }

      //////////////////////////////////////////////////
      //              Sampling Algorithms             //
      //////////////////////////////////////////////////

      if (parser.arg("method")->arg("sample")) {
        // Check timing
        clock_t start_check = clock();

        double init_log_prob;
        Eigen::VectorXd init_grad = Eigen::VectorXd::Zero(model.num_params_r());

        stan::model::gradient(model, cont_params, init_log_prob,
                              init_grad, &std::cout);

        clock_t end_check = clock();
        double deltaT
          = static_cast<double>(end_check - start_check) / CLOCKS_PER_SEC;

        std::cout << std::endl;
        std::cout << "Gradient evaluation took " << deltaT
                  << " seconds" << std::endl;
        std::cout << "1000 transitions using 10 leapfrog steps "
                  << "per transition would take "
                  << 1e4 * deltaT << " seconds." << std::endl;
        std::cout << "Adjust your expectations accordingly!"
                  << std::endl << std::endl;
        std::cout << std::endl;

        stan::services::sample::mcmc_writer
          writer(sample_writer, diagnostic_writer, info);

        // Sampling parameters
        int num_warmup = dynamic_cast<stan::services::int_argument*>(
                          parser.arg("method")->arg("sample")->arg("num_warmup"))->value();

        int num_samples = dynamic_cast<stan::services::int_argument*>(
                          parser.arg("method")->arg("sample")->arg("num_samples"))->value();

        int num_thin = dynamic_cast<stan::services::int_argument*>(
                       parser.arg("method")->arg("sample")->arg("thin"))->value();

        bool save_warmup = dynamic_cast<stan::services::bool_argument*>(
                           parser.arg("method")->arg("sample")->arg("save_warmup"))->value();

        stan::mcmc::sample s(cont_params, 0, 0);

        double warmDeltaT;
        double sampleDeltaT;

        // Sampler
        stan::mcmc::base_mcmc* sampler_ptr = 0;

        stan::services::list_argument* algo
          = dynamic_cast<stan::services::list_argument*>
            (parser.arg("method")->arg("sample")->arg("algorithm"));

        stan::services::categorical_argument* adapt
          = dynamic_cast<stan::services::categorical_argument*>
            (parser.arg("method")->arg("sample")->arg("adapt"));
        bool adapt_engaged
          = dynamic_cast<stan::services::bool_argument*>(adapt->arg("engaged"))
            ->value();

        if (model.num_params_r() == 0 && algo->value() != "fixed_param") {
          std::cout
            << "Must use algorithm=fixed_param for "
            << "model that has no parameters."
            << std::endl;
          return -1;
        }

        if (algo->value() == "fixed_param") {
          return stan::services::sample::fixed_param(model,
                                                     init_context,
                                                     random_seed,
                                                     id,
                                                     init_radius,
                                                     num_samples,
                                                     num_thin,
                                                     refresh,
                                                     interrupt,
                                                     info,
                                                     err,
                                                     sample_writer,
                                                     diagnostic_writer);
        } else if (algo->value() == "rwm") {
          std::cout << algo->arg("rwm")->description() << std::endl;
          return 0;

        } else if (algo->value() == "hmc") {
          stan::services::list_argument* engine
            = dynamic_cast<stan::services::list_argument*>
            (algo->arg("hmc")->arg("engine"));
          stan::services::list_argument* metric
            = dynamic_cast<stan::services::list_argument*>
            (algo->arg("hmc")->arg("metric"));
          stan::services::categorical_argument* adapt
            = dynamic_cast<stan::services::categorical_argument*>
            (parser.arg("method")->arg("sample")->arg("adapt"));
          bool adapt_engaged
            = dynamic_cast<stan::services::bool_argument*>(adapt->arg("engaged"))
            ->value();
          int num_warmup = dynamic_cast<stan::services::int_argument*>(
                          parser.arg("method")->arg("sample")->arg("num_warmup"))->value();
          int num_samples = dynamic_cast<stan::services::int_argument*>(
                          parser.arg("method")->arg("sample")->arg("num_samples"))->value();
          int num_thin = dynamic_cast<stan::services::int_argument*>(
                       parser.arg("method")->arg("sample")->arg("thin"))->value();
          bool save_warmup = dynamic_cast<stan::services::bool_argument*>(
                       parser.arg("method")->arg("sample")->arg("save_warmup"))->value();
          stan::services::categorical_argument* hmc
            = dynamic_cast<stan::services::categorical_argument*>
            (algo->arg("hmc"));
          double stepsize
            = dynamic_cast<stan::services::real_argument*>
            (hmc->arg("stepsize"))->value();
          double stepsize_jitter
            = dynamic_cast<stan::services::real_argument*>
            (hmc->arg("stepsize_jitter"))->value();

          if (engine->value() == "nuts"
              && metric->value() == "dense_e"
              && adapt_engaged == false) {
            int max_depth
              = dynamic_cast<stan::services::int_argument*>
              (dynamic_cast<stan::services::categorical_argument*>
               (algo->arg("hmc")->arg("engine")->arg("nuts"))->arg("max_depth"))
              ->value();

            return stan::services::sample::hmc_nuts_dense_e(model,
                                                            init_context,
                                                            random_seed,
                                                            id,
                                                            init_radius,
                                                            num_warmup,
                                                            num_samples,
                                                            num_thin,
                                                            save_warmup,
                                                            refresh,
                                                            stepsize,
                                                            stepsize_jitter,
                                                            max_depth,
                                                            interrupt,
                                                            info,
                                                            err,
                                                            sample_writer,
                                                            diagnostic_writer);
          } else if (engine->value() == "nuts"
                     && metric->value() == "dense_e"
                     && adapt_engaged == true) {
            int max_depth
              = dynamic_cast<stan::services::int_argument*>
              (dynamic_cast<stan::services::categorical_argument*>
               (algo->arg("hmc")->arg("engine")->arg("nuts"))->arg("max_depth"))
              ->value();
            stan::services::categorical_argument* adapt
              = dynamic_cast<stan::services::categorical_argument*>
              (parser.arg("method")->arg("sample")->arg("adapt"));

            double delta
              = dynamic_cast<real_argument*>(adapt->arg("delta"))->value();
            double gamma
              = dynamic_cast<real_argument*>(adapt->arg("gamma"))->value();
            double kappa
              = dynamic_cast<real_argument*>(adapt->arg("kappa"))->value();
            double t0
              = dynamic_cast<real_argument*>(adapt->arg("t0"))->value();

            unsigned int init_buffer
              = dynamic_cast<u_int_argument*>(adapt->arg("init_buffer"))->value();
            unsigned int term_buffer
              = dynamic_cast<u_int_argument*>(adapt->arg("term_buffer"))->value();
            unsigned int window
              = dynamic_cast<u_int_argument*>(adapt->arg("window"))->value();
                                   
            return stan::services::sample::hmc_nuts_dense_e_adapt(model,
                                                                  init_context,
                                                                  random_seed,
                                                                  id,
                                                                  init_radius,
                                                                  num_warmup,
                                                                  num_samples,
                                                                  num_thin,
                                                                  save_warmup,
                                                                  refresh,
                                                                  stepsize,
                                                                  stepsize_jitter,
                                                                  max_depth,
                                                                  delta,
                                                                  gamma,
                                                                  kappa,
                                                                  t0,
                                                                  init_buffer,
                                                                  term_buffer,
                                                                  window,
                                                                  interrupt,
                                                                  info,
                                                                  err,
                                                                  sample_writer,
                                                                  diagnostic_writer);
          } else if (engine->value() == "nuts"
                     && metric->value() == "diag_e"
                     && adapt_engaged == false) {
          } else if (engine->value() == "nuts"
                     && metric->value() == "diag_e"
                     && adapt_engaged == true) {
          } else if (engine->value() == "nuts"
                     && metric->value() == "unit_e"
                     && adapt_engaged == false) {
          } else if (engine->value() == "nuts"
                     && metric->value() == "unit_e"
                     && adapt_engaged == true) {
          } else if (engine->value() == "static"
                     && metric->value() == "dense_e"
                     && adapt_engaged == false) {
          } else if (engine->value() == "static"
                     && metric->value() == "dense_e"
                     && adapt_engaged == true) {
          } else if (engine->value() == "static"
                     && metric->value() == "diag_e"
                     && adapt_engaged == false) {
          } else if (engine->value() == "static"
                     && metric->value() == "diag_e"
                     && adapt_engaged == true) {
          } else if (engine->value() == "static"
                     && metric->value() == "unit_e"
                     && adapt_engaged == false) {
          } else if (engine->value() == "static"
                     && metric->value() == "unit_e"
                     && adapt_engaged == true) {
          } else {
            // something wrong
          }
          
          /*
          int engine_index = 0;

          stan::services::list_argument* engine
            = dynamic_cast<stan::services::list_argument*>
              (algo->arg("hmc")->arg("engine"));

          if (engine->value() == "static") {
            engine_index = 0;
          } else if (engine->value() == "nuts") {
            engine_index = 1;
          }

          int metric_index = 0;
          stan::services::list_argument* metric
            = dynamic_cast<stan::services::list_argument*>
              (algo->arg("hmc")->arg("metric"));
          if (metric->value() == "unit_e") {
            metric_index = 0;
          } else if (metric->value() == "diag_e") {
            metric_index = 1;
          } else if (metric->value() == "dense_e") {
            metric_index = 2;
          }

          int sampler_select = engine_index
            + 10 * metric_index
            + 100 * static_cast<int>(adapt_engaged);

          switch (sampler_select) {
            case 0: {
              typedef stan::mcmc::unit_e_static_hmc<Model, rng_t> sampler;
              sampler_ptr = new sampler(model, base_rng);
              if (!sample::init_static_hmc<sampler>(sampler_ptr, algo))
                return 0;
              break;
            }

            case 1: {
              typedef stan::mcmc::unit_e_nuts<Model, rng_t> sampler;
              sampler_ptr = new sampler(model, base_rng);
              if (!sample::init_nuts<sampler>(sampler_ptr, algo))
                return 0;
              break;
            }

            case 10: {
              typedef stan::mcmc::diag_e_static_hmc<Model, rng_t> sampler;
              sampler_ptr = new sampler(model, base_rng);
              if (!sample::init_static_hmc<sampler>(sampler_ptr, algo))
                return 0;
              break;
            }

            case 11: {
              typedef stan::mcmc::diag_e_nuts<Model, rng_t> sampler;
              sampler_ptr = new sampler(model, base_rng);
              if (!sample::init_nuts<sampler>(sampler_ptr, algo))
                return 0;
              break;
            }

            case 20: {
              typedef stan::mcmc::dense_e_static_hmc<Model, rng_t> sampler;
              sampler_ptr = new sampler(model, base_rng);
              if (!sample::init_static_hmc<sampler>(sampler_ptr, algo))
                return 0;
              break;
            }

            case 21: {
              typedef stan::mcmc::dense_e_nuts<Model, rng_t> sampler;
              sampler_ptr = new sampler(model, base_rng);
              if (!sample::init_nuts<sampler>(sampler_ptr, algo))
                return 0;
              break;
            }

            case 100: {
              typedef stan::mcmc::adapt_unit_e_static_hmc<Model, rng_t> sampler;
              sampler_ptr = new sampler(model, base_rng);
              if (!sample::init_static_hmc<sampler>(sampler_ptr, algo))
                return 0;
              if (!sample::init_adapt<sampler>(sampler_ptr,
                                               adapt, cont_params,
                                               info, err))
                return 0;
              break;
            }

            case 101: {
              typedef stan::mcmc::adapt_unit_e_nuts<Model, rng_t> sampler;
              sampler_ptr = new sampler(model, base_rng);
              if (!sample::init_nuts<sampler>(sampler_ptr, algo))
                return 0;
              if (!sample::init_adapt<sampler>(sampler_ptr,
                                               adapt, cont_params,
                                               info, err))
                return 0;
              break;
            }

            case 110: {
              typedef stan::mcmc::adapt_diag_e_static_hmc<Model, rng_t> sampler;
              sampler_ptr = new sampler(model, base_rng);
              if (!sample::init_static_hmc<sampler>(sampler_ptr, algo))
                return 0;
              if (!sample::init_windowed_adapt<sampler>(sampler_ptr, adapt, num_warmup,
                                                        cont_params, info, err))
                return 0;
              break;
            }

            case 111: {
              typedef stan::mcmc::adapt_diag_e_nuts<Model, rng_t> sampler;
              sampler_ptr = new sampler(model, base_rng);
              if (!sample::init_nuts<sampler>(sampler_ptr, algo))
                return 0;
              if (!sample::init_windowed_adapt<sampler>(sampler_ptr, adapt, num_warmup,
                                                        cont_params, info, err))
                return 0;
              break;
            }

            case 120: {
              typedef stan::mcmc::adapt_dense_e_static_hmc<Model, rng_t>
                sampler;
              sampler_ptr = new sampler(model, base_rng);
              if (!sample::init_static_hmc<sampler>(sampler_ptr, algo))
                return 0;
              if (!sample::init_windowed_adapt<sampler>(sampler_ptr, adapt, num_warmup,
                                                        cont_params, info, err))
                return 0;
              break;
            }

            case 121: {
              typedef stan::mcmc::adapt_dense_e_nuts<Model, rng_t> sampler;
              sampler_ptr = new sampler(model, base_rng);
              if (!sample::init_nuts<sampler>(sampler_ptr, algo))
                return 0;
              if (!sample::init_windowed_adapt<sampler>(sampler_ptr, adapt, num_warmup,
                                                        cont_params, info, err))
                return 0;
              break;
            }

            default:
              std::cout << "No sampler matching HMC specification!"
                        << std::endl;
              return 0;
          }
          */
        }
        /*
        // Headers
        writer.write_sample_names(s, *sampler_ptr, model);
        writer.write_diagnostic_names(s, *sampler_ptr, model);

        std::string prefix = "";
        std::string suffix = "\n";

        // Warm-Up
        clock_t start = clock();


        mcmc::warmup<Model, rng_t>(*sampler_ptr, num_warmup, num_samples, num_thin,
                                   refresh, save_warmup,
                                   writer,
                                   s, model, base_rng,
                                   prefix, suffix, std::cout,
                                   interrupt,
                                   info, err);

        clock_t end = clock();
        warmDeltaT = static_cast<double>(end - start) / CLOCKS_PER_SEC;

        if (adapt_engaged) {
          dynamic_cast<stan::mcmc::base_adapter*>(sampler_ptr)
            ->disengage_adaptation();
          writer.write_adapt_finish(*sampler_ptr);
        }

        // Sampling
        start = clock();

        mcmc::sample<Model, rng_t>
          (*sampler_ptr, num_warmup, num_samples, num_thin,
           refresh, true,
           writer,
           s, model, base_rng,
           prefix, suffix, std::cout,
           interrupt,
           info, err);

        end = clock();
        sampleDeltaT = static_cast<double>(end - start) / CLOCKS_PER_SEC;

        writer.write_timing(warmDeltaT, sampleDeltaT);

        if (sampler_ptr)
          delete sampler_ptr;
        */
        return 0;
      }

      //////////////////////////////////////////////////
      //           VARIATIONAL Algorithms             //
      //////////////////////////////////////////////////


      if (parser.arg("method")->arg("variational")) {
        stan::services::list_argument* algo
          = dynamic_cast<stan::services::list_argument*>(parser.arg("method")
            ->arg("variational")->arg("algorithm"));

        int grad_samples = dynamic_cast<stan::services::int_argument*>
          (parser.arg("method")->arg("variational")
                               ->arg("grad_samples"))->value();

        int elbo_samples = dynamic_cast<stan::services::int_argument*>
          (parser.arg("method")->arg("variational")
                               ->arg("elbo_samples"))->value();

        int max_iterations = dynamic_cast<stan::services::int_argument*>
          (parser.arg("method")->arg("variational")
                               ->arg("iter"))->value();

        double tol_rel_obj = dynamic_cast<stan::services::real_argument*>
          (parser.arg("method")->arg("variational")
                               ->arg("tol_rel_obj"))->value();

        double eta = dynamic_cast<stan::services::real_argument*>
          (parser.arg("method")->arg("variational")
                               ->arg("eta"))->value();

        bool adapt_engaged = dynamic_cast<stan::services::bool_argument*>
          (parser.arg("method")->arg("variational")->arg("adapt")
                                                   ->arg("engaged"))->value();

        int adapt_iterations = dynamic_cast<stan::services::int_argument*>
          (parser.arg("method")->arg("variational")->arg("adapt")
                                                   ->arg("iter"))->value();

        int eval_elbo = dynamic_cast<stan::services::int_argument*>
          (parser.arg("method")->arg("variational")
                               ->arg("eval_elbo"))->value();

        int output_samples = dynamic_cast<stan::services::int_argument*>
          (parser.arg("method")->arg("variational")
                               ->arg("output_samples"))->value();

        // Check timing
        clock_t start_check = clock();

        double init_log_prob;
        Eigen::VectorXd init_grad
          = Eigen::VectorXd::Zero(model.num_params_r());

        stan::model::gradient(model, cont_params, init_log_prob,
                              init_grad, &std::cout);

        clock_t end_check = clock();
        double deltaT
          = static_cast<double>(end_check - start_check) / CLOCKS_PER_SEC;

        std::cout << std::endl;
        std::cout << "This is Automatic Differentiation Variational Inference.";
        std::cout << std::endl;

        std::cout << std::endl;
        std::cout << "(EXPERIMENTAL ALGORITHM: expect frequent updates to the"
                  << " procedure.)";
        std::cout << std::endl;

        std::cout << std::endl;
        std::cout << "Gradient evaluation took " << deltaT
                  << " seconds" << std::endl;
        std::cout << "1000 iterations under these settings should take "
                  << 1e3 * grad_samples * deltaT << " seconds." << std::endl;
        std::cout << "Adjust your expectations accordingly!";
        std::cout << std::endl;
        std::cout << std::endl;

        if (algo->value() == "fullrank") {
          std::vector<std::string> names;
          names.push_back("lp__");
          model.constrained_param_names(names, true, true);

          sample_writer(names);

          stan::variational::advi<Model,
                                  stan::variational::normal_fullrank,
                                  rng_t>
            cmd_advi(model,
                     cont_params,
                     base_rng,
                     grad_samples,
                     elbo_samples,
                     eval_elbo,
                     output_samples);
          cmd_advi.run(eta, adapt_engaged, adapt_iterations,
                       tol_rel_obj, max_iterations,
                       info, sample_writer, diagnostic_writer);
        }

        if (algo->value() == "meanfield") {
          std::vector<std::string> names;
          names.push_back("lp__");
          model.constrained_param_names(names, true, true);
          sample_writer(names);
          
          stan::variational::advi<Model,
                                  stan::variational::normal_meanfield,
                                  rng_t>
            cmd_advi(model,
                     cont_params,
                     base_rng,
                     grad_samples,
                     elbo_samples,
                     eval_elbo,
                     output_samples);
          cmd_advi.run(eta, adapt_engaged, adapt_iterations,
                       tol_rel_obj, max_iterations,
                       info, sample_writer, diagnostic_writer);
        }
      }

      if (output_stream) {
        output_stream->close();
        delete output_stream;
      }

      if (diagnostic_stream) {
        diagnostic_stream->close();
        delete diagnostic_stream;
      }

      for (size_t i = 0; i < valid_arguments.size(); ++i)
        delete valid_arguments.at(i);

      return 0;
    }

  }  // namespace services
}  // namespace stan

#endif
