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

#include <stan/services/arguments/arg_diagnose.hpp>

#include <stan/services/arguments/arg_method.hpp>
#include <stan/services/arguments/arg_optimize.hpp>
#include <stan/services/arguments/arg_sample.hpp>
#include <stan/services/arguments/argument_parser.hpp>
#include <stan/services/arguments/categorical_argument.hpp>
#include <stan/services/arguments/list_argument.hpp>
#include <stan/services/arguments/singleton_argument.hpp>
#include <stan/services/arguments/unvalued_argument.hpp>
#include <stan/services/arguments/valued_argument.hpp>
#include <stan/services/arguments/arg_variational.hpp>

#include <stan/mcmc/fixed_param_sampler.hpp>
#include <stan/mcmc/hmc/static/adapt_unit_e_static_hmc.hpp>
#include <stan/mcmc/hmc/static/adapt_diag_e_static_hmc.hpp>
#include <stan/mcmc/hmc/static/adapt_dense_e_static_hmc.hpp>
#include <stan/mcmc/hmc/nuts/adapt_unit_e_nuts.hpp>
#include <stan/mcmc/hmc/nuts/adapt_diag_e_nuts.hpp>
#include <stan/mcmc/hmc/nuts/adapt_dense_e_nuts.hpp>

#include <stan/model/util.hpp>

#include <stan/services/diagnose/diagnose.hpp>
#include <stan/services/init/initialize_state.hpp>
#include <stan/services/io/do_print.hpp>
#include <stan/services/io/write_error_msg.hpp>
#include <stan/services/io/write_iteration.hpp>
#include <stan/services/io/write_model.hpp>
#include <stan/services/io/write_stan.hpp>
#include <stan/services/sample/init_adapt.hpp>
#include <stan/services/sample/init_nuts.hpp>
#include <stan/services/sample/init_static_hmc.hpp>
#include <stan/services/sample/init_windowed_adapt.hpp>
#include <stan/services/sample/mcmc_writer.hpp>
#include <stan/services/sample/progress.hpp>
#include <stan/services/sample/run_adaptive_sampler.hpp>
#include <stan/services/sample/run_sampler.hpp>
#include <stan/services/sample/generate_transitions.hpp>
#include <stan/services/sample/sample.hpp>
#include <stan/services/optimize/do_bfgs_optimize.hpp>
#include <stan/services/optimize/optimize.hpp>
//#include <stan/services/variational/variational.hpp>

// FIXME: These belong to the interfaces and should be templated out here
#include <stan/interface_callbacks/interrupt/noop.hpp>
#include <stan/interface_callbacks/var_context_factory/dump_factory.hpp>
#include <stan/interface_callbacks/writer/stream_writer.hpp>

#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace stan {
  namespace services {

    template <class Model>
    int command(int argc, const char* argv[]) {
      
      typedef
        stan::interface_callbacks::writer::stream_writer<std::ostream>
          ostream_writer;

      stan::interface_callbacks::interrupt::noop iteration_interrupt;

      ostream_writer info(std::cout); // Informative messages
      ostream_writer err(std::cerr);  // Error messages

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

      // Sample output
      std::string output_file =
        dynamic_cast<string_argument*>(parser.arg("output")->arg("file"))->value();
      std::fstream output_stream;
      output_stream.open(output_file.c_str(), std::fstream::out);
      if (!output_stream.good()) {
        info("Could not open the output file " + output_file);
        return err_code;
      }
      
      ostream_writer output_writer(output_stream);

      // Diagnostic output
      std::string diagnostic_file =
        dynamic_cast<string_argument*>(parser.arg("output")->arg("diagnostic_file"))->value();
      std::fstream diagnostic_stream;
      diagnostic_stream.open(diagnostic_file.c_str(), std::fstream::out);
      if (!(diagnostic_stream.good() || diagnostic_file == "")) {
        info("Could not open the diagnostic file " + diagnostic_file);
        return err_code;
      }
      
      ostream_writer diagnostic_writer(diagnostic_stream);

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

      services::io::write_stan(output_writer, "#");
      services::io::write_model(output_writer, model.model_name(), "#");
      parser.print(output_writer, "#");

      services::io::write_stan(diagnostic_writer, "#");
      services::io::write_model(diagnostic_writer, model.model_name(), "#");
      parser.print(diagnostic_writer, "#");

      std::string init = dynamic_cast<stan::services::string_argument*>(
                         parser.arg("init"))->value();

      interface_callbacks::var_context_factory::dump_factory var_context_factory;
      if (!init::initialize_state<Model, rng_t, ostream_writer,
                                  interface_callbacks::var_context_factory::dump_factory>
          (init, cont_params, model, base_rng, info, var_context_factory))
        return stan::services::error_codes::SOFTWARE;

      //////////////////////////////////////////////////
      //               Model Diagnostics              //
      //////////////////////////////////////////////////

      if (parser.arg("method")->arg("diagnose")) {
        stan::services::list_argument* test = dynamic_cast<stan::services::list_argument*>
                                              (parser.arg("method")->arg("diagnose")->arg("test"));
        return diagnose::diagnose(cont_params, model, test, info, output_writer);
      }

      //////////////////////////////////////////////////
      //             Sampling Algorithms              //
      //////////////////////////////////////////////////
      
      if (parser.arg("method")->arg("sample")) {
        stan::services::categorical_argument* sample_args
          = dynamic_cast<stan::services::categorical_argument*>
              (parser.arg("method")->arg("sample"));
        return sample::sample(cont_params, model, base_rng, sample_args, refresh,
                              info, err, output_writer, diagnostic_writer,
                              iteration_interrupt);
        
      }
      
      //////////////////////////////////////////////////
      //           Optimization Algorithms            //
      //////////////////////////////////////////////////

      if (parser.arg("method")->arg("optimize")) {
        stan::services::categorical_argument* optimize_args
          = dynamic_cast<stan::services::categorical_argument*>
              (parser.arg("method")->arg("optimize"));
        return optimize::optimize(cont_params, model, base_rng, optimize_args, refresh,
                                  info, err, output_writer, iteration_interrupt);

      }

      //////////////////////////////////////////////////
      //           Variational Algorithms             //
      //////////////////////////////////////////////////

      /*
      if (parser.arg("method")->arg("variational")) {
        stan::services::categorical_argument* variational_args
          = dynamic_cast<stan::services::categorical_argument*>
              (parser.arg("method")->arg("variational"));
        return varitional::variational(cont_params, model, base_rng, sample_args, refresh,
                                       output_stream, diagnostic_stream,
                                       iteration_interrupt);
      }
      */
      return stan::services::error_codes::USAGE;

    }

  }  // namespace services
}  // namespace stan

#endif
