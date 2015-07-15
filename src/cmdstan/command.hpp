#ifndef CMDSTAN_COMMAND_HPP
#define CMDSTAN_COMMAND_HPP

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/random/additive_combine.hpp>  // L'Ecuyer RNG
#include <boost/random/uniform_real_distribution.hpp>

#include <stan/version.hpp>
#include <stan/io/cmd_line.hpp>
#include <stan/io/dump.hpp>
#include <stan/io/json.hpp>

#include <stan/services/arguments.hpp>

#include <stan/model/util.hpp>

#include <stan/services/diagnose.hpp>
#include <stan/services/init.hpp>
#include <stan/services/io.hpp>
#include <stan/services/optimize.hpp>
#include <stan/services/sample.hpp>

#include <stan/variational/advi.hpp>

// FIXME: These belong to the interfaces and should be templated out here
#include <stan/interface_callbacks/interrupt/noop.hpp>
#include <stan/interface_callbacks/var_context_factory/dump_factory.hpp>
#include <stan/interface_callbacks/writer.hpp>

#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace stan {
  namespace services {

      // BEGIN TEMP CALLBACKS
      // FIXME: The below should all be callbacks created externally

      stan::interface_callbacks::interrupt::noop iteration_interrupt;

      stan::interface_callbacks::writer::cout info; // Informative messages
      stan::interface_callbacks::writer::cerr err;  // Error messages

      // END TEMP CALLBACKS

      argument_parser parser;
      parser.push_valid_arg(new arg_id());
      parser.push_valid_arg(new stan::services::arg_data());
      parser.push_valid_arg(new stan::services::arg_init());
      parser.push_valid_arg(new stan::services::arg_random());
      parser.push_valid_arg(new stan::services::arg_output());

      int err_code = parser.parse_args(argc, argv, info, err);

      if (err_code != 0) {
        info("Failed to parse arguments, terminating Stan");
        return err_code;
      }

      if (parser.help_printed())
        return err_code;

      // BEGIN TEMP CALLBACKS
      // FIXME: The below should all be callbacks created externally

      // Sample output
      std::string output_file =
        dynamic_cast<string_argument*>(parser.arg("output")->arg("file"))->value();
      stan::interface_callbacks::writer::fstream_csv output_stream(output_file);

      // Diagnostic output
      std::string diagnostic_file =
        dynamic_cast<string_argument*>(parser.arg("output")->arg("diagnostic_file"))->value();
      stan::interface_callbacks::writer::fstream_csv diagnostic_stream(diagnostic_file);

      // END TEMP CALLBACKS

      // Identification
      unsigned int id = dynamic_cast<stan::services::int_argument*>
        (parser.arg("id"))->value();

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

      typedef boost::ecuyer1988 rng_t;  // (2**50 = 1T samples, 1000 chains)
      rng_t base_rng(random_seed);

      // Advance generator to avoid process conflicts
      static boost::uintmax_t DISCARD_STRIDE
        = static_cast<boost::uintmax_t>(1) << 50;
      base_rng.discard(DISCARD_STRIDE * (id - 1));

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

      // Refresh rate
      int refresh = dynamic_cast<stan::services::int_argument*>
                    (parser.arg("output")->arg("refresh"))->value();

      //////////////////////////////////////////////////
      //                Initialize Model              //
      //////////////////////////////////////////////////

      std::stringstream model_output;
      Model model(data_var_context, &model_output);
      if (model_output.str().size()) info(model_output.str());

      Eigen::VectorXd cont_params = Eigen::VectorXd::Zero(model.num_params_r());

      parser.print(info);
      info();

      services::io::write_stan(output_stream, "#");
      services::io::write_model(output_stream, model.model_name(), "#");
      parser.print(output_stream, "#");

      services::io::write_stan(diagnostic_stream, "#");
      services::io::write_model(diagnostic_stream, model.model_name(), "#");
      parser.print(diagnostic_stream, "#");

      std::string init = dynamic_cast<stan::services::string_argument*>(
                         parser.arg("init"))->value();

      interface_callbacks::var_context_factory::dump_factory var_context_factory;
      if (!init::initialize_state<interface_callbacks::var_context_factory::dump_factory>
          (init, cont_params, model, base_rng, info, var_context_factory))
        return stan::services::error_codes::SOFTWARE;

      //////////////////////////////////////////////////
      //               Model Diagnostics              //
      //////////////////////////////////////////////////

      if (parser.arg("method")->arg("diagnose")) {
        stan::services::list_argument* test = dynamic_cast<stan::services::list_argument*>
                                              (parser.arg("method")->arg("diagnose")->arg("test"));
        return diagnose::diagnose(cont_params, model, test, info, output_stream);
      }

      //////////////////////////////////////////////////
      //           Optimization Algorithms            //
      //////////////////////////////////////////////////

      if (parser.arg("method")->arg("optimize")) {
        stan::services::categorical_argument* optimize_args = dynamic_cast<stan::services::categorical_argument*>
                                                              (parser.arg("method")->arg("optimize"));
        return optimize::optimize(cont_params, model, base_rng, optimize_args, refresh,
                                  info, err, output_stream, iteration_interrupt);
      }

      //////////////////////////////////////////////////
      //              Sampling Algorithms             //
      //////////////////////////////////////////////////

      if (parser.arg("method")->arg("sample")) {
        stan::services::categorical_argument* sample_args = dynamic_cast<stan::services::categorical_argument*>
                                                     (parser.arg("method")->arg("sample"));
        sample::sample(cont_params, model, base_rng, sample_args, refresh,
                       info, err, output_stream, diagnostic_stream,
                       iteration_interrupt);

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

        double eta_adagrad = dynamic_cast<stan::services::real_argument*>
          (parser.arg("method")->arg("variational")
                               ->arg("eta_adagrad"))->value();

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
          if (output_stream) {
            std::vector<std::string> names;
            names.push_back("lp");
            model.constrained_param_names(names, true, true);

            (*output_stream) << names.at(0);
            for (size_t i = 1; i < names.size(); ++i) {
              (*output_stream) << "," << names.at(i);
            }
            (*output_stream) << std::endl;
          }

          stan::variational::advi<Model,
                                  stan::variational::normal_fullrank,
                                  rng_t>
            cmd_advi(model,
                     cont_params,
                     grad_samples,
                     elbo_samples,
                     eta_adagrad,
                     base_rng,
                     eval_elbo,
                     output_samples,
                     &std::cout,
                     output_stream,
                     diagnostic_stream);
          cmd_advi.run(tol_rel_obj, max_iterations);
        }

        if (algo->value() == "meanfield") {
          if (output_stream) {
            std::vector<std::string> names;
            names.push_back("lp");
            model.constrained_param_names(names, true, true);

            (*output_stream) << names.at(0);
            for (size_t i = 1; i < names.size(); ++i) {
              (*output_stream) << "," << names.at(i);
            }
            (*output_stream) << std::endl;
          }

          stan::variational::advi<Model,
                                  stan::variational::normal_meanfield,
                                  rng_t>
            cmd_advi(model,
                     cont_params,
                     grad_samples,
                     elbo_samples,
                     eta_adagrad,
                     base_rng,
                     eval_elbo,
                     output_samples,
                     &std::cout,
                     output_stream,
                     diagnostic_stream);
          cmd_advi.run(tol_rel_obj, max_iterations);
        }
      }

      return stan::services::error_codes::USAGE;

    }

  }  // namespace services
}  // namespace stan

#endif
