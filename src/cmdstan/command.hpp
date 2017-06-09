#ifndef CMDSTAN_COMMAND_HPP
#define CMDSTAN_COMMAND_HPP

#include <cmdstan/arguments/argument_parser.hpp>
#include <cmdstan/arguments/arg_data.hpp>
#include <cmdstan/arguments/arg_id.hpp>
#include <cmdstan/arguments/arg_init.hpp>
#include <cmdstan/arguments/arg_output.hpp>
#include <cmdstan/arguments/arg_random.hpp>
#include <cmdstan/write_model.hpp>
#include <cmdstan/write_stan.hpp>
#include <stan/callbacks/interrupt.hpp>
#include <stan/callbacks/logger.hpp>
#include <stan/callbacks/stream_logger.hpp>
#include <stan/callbacks/stream_writer.hpp>
#include <stan/callbacks/writer.hpp>
#include <stan/io/dump.hpp>
#include <stan/services/diagnose/diagnose.hpp>
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
#include <stan/services/experimental/advi/fullrank.hpp>
#include <stan/services/experimental/advi/meanfield.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cmdstan {

  stan::io::dump get_var_context(const std::string file) {
    std::fstream stream(file.c_str(), std::fstream::in);
    if (file != "" && (stream.rdstate() & std::ifstream::failbit)) {
      std::stringstream msg;
      msg << "Can't open specified file, \"" << file << "\"" << std::endl;
      throw std::invalid_argument(msg.str());
    }
    stan::io::dump var_context(stream);
    stream.close();
    return var_context;
  }

  template <class Model>
  int command(int argc, const char* argv[]) {
    stan::callbacks::stream_writer info(std::cout);
    stan::callbacks::stream_writer err(std::cout);
    stan::callbacks::stream_logger logger(std::cout, std::cout, std::cout,
                                          std::cerr, std::cerr);

    // Read arguments
    std::vector<argument*> valid_arguments;
    valid_arguments.push_back(new arg_id());
    valid_arguments.push_back(new arg_data());
    valid_arguments.push_back(new arg_init());
    valid_arguments.push_back(new arg_random());
    valid_arguments.push_back(new arg_output());
    argument_parser parser(valid_arguments);
    int err_code = parser.parse_args(argc, argv, info, err);
    if (err_code != 0) {
      std::cout << "Failed to parse arguments, terminating Stan" << std::endl;
      return err_code;
    }
    if (parser.help_printed())
      return err_code;
    u_int_argument* random_arg = dynamic_cast<u_int_argument*>(parser.arg("random")->arg("seed"));
    if (random_arg->is_default()) {
      random_arg->set_value((boost::posix_time::microsec_clock::universal_time() - boost::posix_time::ptime(boost::posix_time::min_date_time)).total_milliseconds());
    }
    parser.print(info);
    info();


    stan::callbacks::writer init_writer;
    stan::callbacks::interrupt interrupt;

    stan::io::dump data_var_context(get_var_context(dynamic_cast<string_argument*>(parser.arg("data")->arg("file"))->value()));

    std::fstream output_stream(dynamic_cast<string_argument*>(parser.arg("output")->arg("file"))->value().c_str(),
                               std::fstream::out);
    stan::callbacks::stream_writer sample_writer(output_stream, "# ");

    std::fstream diagnostic_stream(dynamic_cast<string_argument*>(parser.arg("output")->arg("diagnostic_file"))->value().c_str(),
                                   std::fstream::out);
    stan::callbacks::stream_writer diagnostic_writer(diagnostic_stream, "# ");


    //////////////////////////////////////////////////
    //                Initialize Model              //
    //////////////////////////////////////////////////
    Model model(data_var_context, &std::cout);
    write_stan(sample_writer);
    write_model(sample_writer, model.model_name());
    parser.print(sample_writer);

    write_stan(diagnostic_writer);
    write_model(diagnostic_writer, model.model_name());
    parser.print(diagnostic_writer);


    int refresh = dynamic_cast<int_argument*>(parser.arg("output")->arg("refresh"))->value();
    unsigned int id = dynamic_cast<int_argument*>(parser.arg("id"))->value();
    unsigned int random_seed = dynamic_cast<u_int_argument*>(parser.arg("random")->arg("seed"))->value();

    std::string init = dynamic_cast<string_argument*>(parser.arg("init"))->value();
    double init_radius = 2.0;
    try {
      init_radius = boost::lexical_cast<double>(init);
      init = "";
    } catch (const boost::bad_lexical_cast& e) {
    }
    stan::io::dump init_context(get_var_context(init));

    int return_code = stan::services::error_codes::CONFIG;
    if (parser.arg("method")->arg("diagnose")) {
      list_argument* test = dynamic_cast<list_argument*>(parser.arg("method")->arg("diagnose")->arg("test"));

      if (test->value() == "gradient") {
        double epsilon = dynamic_cast<real_argument*>(test->arg("gradient")->arg("epsilon"))->value();
        double error = dynamic_cast<real_argument*>(test->arg("gradient")->arg("error"))->value();
        return_code = stan::services::diagnose::diagnose(model,
                                                         init_context,
                                                         random_seed, id,
                                                         init_radius,
                                                         epsilon, error,
                                                         interrupt,
                                                         logger,
                                                         init_writer,
                                                         sample_writer);
      }
    } else if (parser.arg("method")->arg("optimize")) {
      list_argument* algo = dynamic_cast<list_argument*>(parser.arg("method")->arg("optimize")->arg("algorithm"));
      int num_iterations = dynamic_cast<int_argument*>(parser.arg("method")->arg("optimize")->arg("iter"))->value();
      bool save_iterations = dynamic_cast<bool_argument*>(parser.arg("method")->arg("optimize")->arg("save_iterations"))->value();

      if (algo->value() == "newton") {
        return_code = stan::services::optimize::newton(model,
                                                       init_context,
                                                       random_seed,
                                                       id,
                                                       init_radius,
                                                       num_iterations,
                                                       save_iterations,
                                                       interrupt,
                                                       logger,
                                                       init_writer,
                                                       sample_writer);
      } else if (algo->value() == "bfgs") {
        double init_alpha = dynamic_cast<real_argument*>(algo->arg("bfgs")->arg("init_alpha"))->value();
        double tol_obj = dynamic_cast<real_argument*>(algo->arg("bfgs")->arg("tol_obj"))->value();
        double tol_rel_obj = dynamic_cast<real_argument*>(algo->arg("bfgs")->arg("tol_rel_obj"))->value();
        double tol_grad = dynamic_cast<real_argument*>(algo->arg("bfgs")->arg("tol_grad"))->value();
        double tol_rel_grad = dynamic_cast<real_argument*>(algo->arg("bfgs")->arg("tol_rel_grad"))->value();
        double tol_param = dynamic_cast<real_argument*>(algo->arg("bfgs")->arg("tol_param"))->value();

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
                                                     logger,
                                                     init_writer,
                                                     sample_writer);
      } else if (algo->value() == "lbfgs") {
        int history_size = dynamic_cast<int_argument*>(algo->arg("lbfgs")->arg("history_size"))->value();
        double init_alpha = dynamic_cast<real_argument*>(algo->arg("lbfgs")->arg("init_alpha"))->value();
        double tol_obj = dynamic_cast<real_argument*>(algo->arg("lbfgs")->arg("tol_obj"))->value();
        double tol_rel_obj = dynamic_cast<real_argument*>(algo->arg("lbfgs")->arg("tol_rel_obj"))->value();
        double tol_grad = dynamic_cast<real_argument*>(algo->arg("lbfgs")->arg("tol_grad"))->value();
        double tol_rel_grad = dynamic_cast<real_argument*>(algo->arg("lbfgs")->arg("tol_rel_grad"))->value();
        double tol_param = dynamic_cast<real_argument*>(algo->arg("lbfgs")->arg("tol_param"))->value();

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
                                                      logger,
                                                      init_writer,
                                                      sample_writer);
      }
    } else if (parser.arg("method")->arg("sample")) {
      int num_warmup = dynamic_cast<int_argument*>(parser.arg("method")->arg("sample")->arg("num_warmup"))->value();
      int num_samples = dynamic_cast<int_argument*>(parser.arg("method")->arg("sample")->arg("num_samples"))->value();
      int num_thin = dynamic_cast<int_argument*>(parser.arg("method")->arg("sample")->arg("thin"))->value();
      bool save_warmup = dynamic_cast<bool_argument*>(parser.arg("method")->arg("sample")->arg("save_warmup"))->value();
      list_argument* algo = dynamic_cast<list_argument*>(parser.arg("method")->arg("sample")->arg("algorithm"));
      categorical_argument* adapt = dynamic_cast<categorical_argument*>(parser.arg("method")->arg("sample")->arg("adapt"));
      bool adapt_engaged = dynamic_cast<bool_argument*>(adapt->arg("engaged"))->value();

      if (model.num_params_r() == 0 && algo->value() != "fixed_param") {
        info("Must use algorithm=fixed_param for model that has no parameters.");
        return_code = stan::services::error_codes::CONFIG;
      } else if (algo->value() == "fixed_param") {
        return_code = stan::services::sample::fixed_param(model,
                                                          init_context,
                                                          random_seed,
                                                          id,
                                                          init_radius,
                                                          num_samples,
                                                          num_thin,
                                                          refresh,
                                                          interrupt,
                                                          logger,
                                                          init_writer,
                                                          sample_writer,
                                                          diagnostic_writer);
      } else if (algo->value() == "hmc") {
        list_argument* engine = dynamic_cast<list_argument*>(algo->arg("hmc")->arg("engine"));
        list_argument* metric = dynamic_cast<list_argument*>(algo->arg("hmc")->arg("metric"));
        categorical_argument* adapt = dynamic_cast<categorical_argument*>(parser.arg("method")->arg("sample")->arg("adapt"));

        categorical_argument* hmc = dynamic_cast<categorical_argument*>(algo->arg("hmc"));
        double stepsize = dynamic_cast<real_argument*>(hmc->arg("stepsize"))->value();
        double stepsize_jitter= dynamic_cast<real_argument*>(hmc->arg("stepsize_jitter"))->value();

        if (engine->value() == "nuts" && metric->value() == "dense_e" && adapt_engaged == false) {
          int max_depth = dynamic_cast<int_argument*>(dynamic_cast<categorical_argument*>(algo->arg("hmc")->arg("engine")->arg("nuts"))->arg("max_depth"))->value();
          return_code = stan::services::sample::hmc_nuts_dense_e(model,
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
                                                                 logger,
                                                                 init_writer,
                                                                 sample_writer,
                                                                 diagnostic_writer);
        } else if (engine->value() == "nuts" && metric->value() == "dense_e" && adapt_engaged == true) {
          int max_depth = dynamic_cast<int_argument*>(dynamic_cast<categorical_argument*>(algo->arg("hmc")->arg("engine")->arg("nuts"))->arg("max_depth"))->value();
          double delta = dynamic_cast<real_argument*>(adapt->arg("delta"))->value();
          double gamma = dynamic_cast<real_argument*>(adapt->arg("gamma"))->value();
          double kappa = dynamic_cast<real_argument*>(adapt->arg("kappa"))->value();
          double t0 = dynamic_cast<real_argument*>(adapt->arg("t0"))->value();
          unsigned int init_buffer = dynamic_cast<u_int_argument*>(adapt->arg("init_buffer"))->value();
          unsigned int term_buffer = dynamic_cast<u_int_argument*>(adapt->arg("term_buffer"))->value();
          unsigned int window = dynamic_cast<u_int_argument*>(adapt->arg("window"))->value();
          return_code = stan::services::sample::hmc_nuts_dense_e_adapt(model,
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
                                                                       logger,
                                                                       init_writer,
                                                                       sample_writer,
                                                                       diagnostic_writer);
        } else if (engine->value() == "nuts" && metric->value() == "diag_e" && adapt_engaged == false) {
          categorical_argument* base = dynamic_cast<categorical_argument*>(algo->arg("hmc")->arg("engine")->arg("nuts"));
          int max_depth = dynamic_cast<int_argument*>(base->arg("max_depth"))->value();
          return_code = stan::services::sample::hmc_nuts_diag_e(model,
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
                                                                logger,
                                                                init_writer,
                                                                sample_writer,
                                                                diagnostic_writer);
        } else if (engine->value() == "nuts" && metric->value() == "diag_e" && adapt_engaged == true) {
          categorical_argument* base = dynamic_cast<categorical_argument*>(algo->arg("hmc")->arg("engine")->arg("nuts"));
          int max_depth = dynamic_cast<int_argument*>(base->arg("max_depth"))->value();
          double delta = dynamic_cast<real_argument*>(adapt->arg("delta"))->value();
          double gamma = dynamic_cast<real_argument*>(adapt->arg("gamma"))->value();
          double kappa = dynamic_cast<real_argument*>(adapt->arg("kappa"))->value();
          double t0 = dynamic_cast<real_argument*>(adapt->arg("t0"))->value();
          unsigned int init_buffer = dynamic_cast<u_int_argument*>(adapt->arg("init_buffer"))->value();
          unsigned int term_buffer = dynamic_cast<u_int_argument*>(adapt->arg("term_buffer"))->value();
          unsigned int window = dynamic_cast<u_int_argument*>(adapt->arg("window"))->value();
          return_code = stan::services::sample::hmc_nuts_diag_e_adapt(model,
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
                                                                      logger,
                                                                      init_writer,
                                                                      sample_writer,
                                                                      diagnostic_writer);
        } else if (engine->value() == "nuts" && metric->value() == "unit_e" && adapt_engaged == false) {
          categorical_argument* base = dynamic_cast<categorical_argument*>(algo->arg("hmc")->arg("engine")->arg("nuts"));
          int max_depth = dynamic_cast<int_argument*>(base->arg("max_depth"))->value();
          return_code = stan::services::sample::hmc_nuts_unit_e(model,
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
                                                                logger,
                                                                init_writer,
                                                                sample_writer,
                                                                diagnostic_writer);
        } else if (engine->value() == "nuts" && metric->value() == "unit_e" && adapt_engaged == true) {
          categorical_argument* base = dynamic_cast<categorical_argument*>(algo->arg("hmc")->arg("engine")->arg("nuts"));
          int max_depth = dynamic_cast<int_argument*>(base->arg("max_depth"))->value();
          double delta = dynamic_cast<real_argument*>(adapt->arg("delta"))->value();
          double gamma = dynamic_cast<real_argument*>(adapt->arg("gamma"))->value();
          double kappa = dynamic_cast<real_argument*>(adapt->arg("kappa"))->value();
          double t0 = dynamic_cast<real_argument*>(adapt->arg("t0"))->value();
          return_code = stan::services::sample::hmc_nuts_unit_e_adapt(model,
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
                                                                      interrupt,
                                                                      logger,
                                                                      init_writer,
                                                                      sample_writer,
                                                                      diagnostic_writer);
        } else if (engine->value() == "static" && metric->value() == "dense_e" && adapt_engaged == false) {
          categorical_argument* base = dynamic_cast<categorical_argument*>(algo->arg("hmc")->arg("engine")->arg("static"));
          double int_time = dynamic_cast<real_argument*>(base->arg("int_time"))->value();
          return_code = stan::services::sample::hmc_static_dense_e(model,
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
                                                                   int_time,
                                                                   interrupt,
                                                                   logger,
                                                                   init_writer,
                                                                   sample_writer,
                                                                   diagnostic_writer);
        } else if (engine->value() == "static" && metric->value() == "dense_e" && adapt_engaged == true) {
          categorical_argument* base = dynamic_cast<categorical_argument*>(algo->arg("hmc")->arg("engine")->arg("static"));
          double int_time = dynamic_cast<real_argument*>(base->arg("int_time"))->value();
          double delta = dynamic_cast<real_argument*>(adapt->arg("delta"))->value();
          double gamma = dynamic_cast<real_argument*>(adapt->arg("gamma"))->value();
          double kappa = dynamic_cast<real_argument*>(adapt->arg("kappa"))->value();
          double t0 = dynamic_cast<real_argument*>(adapt->arg("t0"))->value();
          unsigned int init_buffer = dynamic_cast<u_int_argument*>(adapt->arg("init_buffer"))->value();
          unsigned int term_buffer = dynamic_cast<u_int_argument*>(adapt->arg("term_buffer"))->value();
          unsigned int window = dynamic_cast<u_int_argument*>(adapt->arg("window"))->value();
          return_code = stan::services::sample::hmc_static_dense_e_adapt(model,
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
                                                                         int_time,
                                                                         delta,
                                                                         gamma,
                                                                         kappa,
                                                                         t0,
                                                                         init_buffer,
                                                                         term_buffer,
                                                                         window,
                                                                         interrupt,
                                                                         logger,
                                                                         init_writer,
                                                                         sample_writer,
                                                                         diagnostic_writer);
        } else if (engine->value() == "static" && metric->value() == "diag_e" && adapt_engaged == false) {
          categorical_argument* base = dynamic_cast<categorical_argument*>(algo->arg("hmc")->arg("engine")->arg("static"));
          double int_time = dynamic_cast<real_argument*>(base->arg("int_time"))->value();
          return_code = stan::services::sample::hmc_static_diag_e(model,
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
                                                                  int_time,
                                                                  interrupt,
                                                                  logger,
                                                                  init_writer,
                                                                  sample_writer,
                                                                  diagnostic_writer);
        } else if (engine->value() == "static" && metric->value() == "diag_e" && adapt_engaged == true) {
          categorical_argument* base = dynamic_cast<categorical_argument*>(algo->arg("hmc")->arg("engine")->arg("static"));
          double int_time = dynamic_cast<real_argument*>(base->arg("int_time"))->value();
          double delta = dynamic_cast<real_argument*>(adapt->arg("delta"))->value();
          double gamma = dynamic_cast<real_argument*>(adapt->arg("gamma"))->value();
          double kappa = dynamic_cast<real_argument*>(adapt->arg("kappa"))->value();
          double t0 = dynamic_cast<real_argument*>(adapt->arg("t0"))->value();
          unsigned int init_buffer = dynamic_cast<u_int_argument*>(adapt->arg("init_buffer"))->value();
          unsigned int term_buffer = dynamic_cast<u_int_argument*>(adapt->arg("term_buffer"))->value();
          unsigned int window = dynamic_cast<u_int_argument*>(adapt->arg("window"))->value();
          return_code = stan::services::sample::hmc_static_diag_e_adapt(model,
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
                                                                        int_time,
                                                                        delta,
                                                                        gamma,
                                                                        kappa,
                                                                        t0,
                                                                        init_buffer,
                                                                        term_buffer,
                                                                        window,
                                                                        interrupt,
                                                                        logger,
                                                                        init_writer,
                                                                        sample_writer,
                                                                        diagnostic_writer);
        } else if (engine->value() == "static" && metric->value() == "unit_e" && adapt_engaged == false) {
          categorical_argument* base = dynamic_cast<categorical_argument*>(algo->arg("hmc")->arg("engine")->arg("static"));
          double int_time = dynamic_cast<real_argument*>(base->arg("int_time"))->value();
          return_code = stan::services::sample::hmc_static_unit_e(model,
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
                                                                  int_time,
                                                                  interrupt,
                                                                  logger,
                                                                  init_writer,
                                                                  sample_writer,
                                                                  diagnostic_writer);
        } else if (engine->value() == "static" && metric->value() == "unit_e" && adapt_engaged == true) {
          categorical_argument* base = dynamic_cast<categorical_argument*>(algo->arg("hmc")->arg("engine")->arg("static"));
          double int_time = dynamic_cast<real_argument*>(base->arg("int_time"))->value();
          double delta = dynamic_cast<real_argument*>(adapt->arg("delta"))->value();
          double gamma = dynamic_cast<real_argument*>(adapt->arg("gamma"))->value();
          double kappa = dynamic_cast<real_argument*>(adapt->arg("kappa"))->value();
          double t0 = dynamic_cast<real_argument*>(adapt->arg("t0"))->value();
          return_code = stan::services::sample::hmc_static_unit_e_adapt(model,
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
                                                                        int_time,
                                                                        delta,
                                                                        gamma,
                                                                        kappa,
                                                                        t0,
                                                                        interrupt,
                                                                        logger,
                                                                        init_writer,
                                                                        sample_writer,
                                                                        diagnostic_writer);
        }
      }
    } else if (parser.arg("method")->arg("variational")) {
      list_argument* algo = dynamic_cast<list_argument*>(parser.arg("method")->arg("variational")->arg("algorithm"));
      int grad_samples = dynamic_cast<int_argument*>(parser.arg("method")->arg("variational")->arg("grad_samples"))->value();
      int elbo_samples = dynamic_cast<int_argument*>(parser.arg("method")->arg("variational")->arg("elbo_samples"))->value();
      int max_iterations = dynamic_cast<int_argument*>(parser.arg("method")->arg("variational")->arg("iter"))->value();
      double tol_rel_obj = dynamic_cast<real_argument*>(parser.arg("method")->arg("variational")->arg("tol_rel_obj"))->value();
      double eta = dynamic_cast<real_argument*>(parser.arg("method")->arg("variational")->arg("eta"))->value();
      bool adapt_engaged = dynamic_cast<bool_argument*>(parser.arg("method")->arg("variational")->arg("adapt")->arg("engaged"))->value();
      int adapt_iterations = dynamic_cast<int_argument*>(parser.arg("method")->arg("variational")->arg("adapt")->arg("iter"))->value();
      int eval_elbo = dynamic_cast<int_argument*>(parser.arg("method")->arg("variational")->arg("eval_elbo"))->value();
      int output_samples = dynamic_cast<int_argument*>(parser.arg("method")->arg("variational")->arg("output_samples"))->value();

      if (algo->value() == "fullrank") {
        return_code = stan::services::experimental::advi::fullrank(model,
                                                                   init_context,
                                                                   random_seed,
                                                                   id,
                                                                   init_radius,
                                                                   grad_samples,
                                                                   elbo_samples,
                                                                   max_iterations,
                                                                   tol_rel_obj,
                                                                   eta,
                                                                   adapt_engaged,
                                                                   adapt_iterations,
                                                                   eval_elbo,
                                                                   output_samples,
                                                                   interrupt,
                                                                   logger,
                                                                   init_writer,
                                                                   sample_writer,
                                                                   diagnostic_writer);
      } else if (algo->value() == "meanfield") {
        return_code = stan::services::experimental::advi::meanfield(model,
                                                                    init_context,
                                                                    random_seed,
                                                                    id,
                                                                    init_radius,
                                                                    grad_samples,
                                                                    elbo_samples,
                                                                    max_iterations,
                                                                    tol_rel_obj,
                                                                    eta,
                                                                    adapt_engaged,
                                                                    adapt_iterations,
                                                                    eval_elbo,
                                                                    output_samples,
                                                                    interrupt,
                                                                    logger,
                                                                    init_writer,
                                                                    sample_writer,
                                                                    diagnostic_writer);
      }
    }

    output_stream.close();
    diagnostic_stream.close();
    for (size_t i = 0; i < valid_arguments.size(); ++i)
      delete valid_arguments.at(i);
    return return_code;
  }

}
#endif
