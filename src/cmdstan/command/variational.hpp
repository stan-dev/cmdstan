#ifndef CMDSTAN_COMMAND_VARIATIONAL_HPP
#define CMDSTAN_COMMAND_VARIATIONAL_HPP

#include <cmdstan/cli.hpp>
#include <cmdstan/write_model.hpp>
#include <cmdstan/write_opencl_device.hpp>
#include <cmdstan/write_parallel_info.hpp>
#include <cmdstan/write_stan.hpp>
#include <cmdstan/command/util.hpp>
#include <stan/callbacks/interrupt.hpp>
#include <stan/callbacks/logger.hpp>
#include <stan/callbacks/stream_logger.hpp>
#include <stan/callbacks/stream_writer.hpp>
#include <stan/callbacks/writer.hpp>
#include <stan/io/dump.hpp>
#include <stan/io/stan_csv_reader.hpp>
#include <stan/io/var_context.hpp>
#include <stan/model/model_base.hpp>
#include <stan/services/experimental/advi/fullrank.hpp>
#include <stan/services/experimental/advi/meanfield.hpp>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace cmdstan {

  int variational(CLI::App& app,
		  SharedOptions& shared_options,
		  VariationalOptions& variational_options) {
    stan::callbacks::stream_writer info(std::cout);
    stan::callbacks::stream_writer err(std::cout);
    stan::callbacks::stream_logger logger(std::cout, std::cout, std::cout,
					  std::cerr, std::cerr);

    // Read arguments
    write_parallel_info(info);
    write_opencl_device(info);
    info();

    stan::callbacks::writer init_writer;
    stan::callbacks::interrupt interrupt;

    std::fstream output_stream(shared_options.output_file.c_str(),
			       std::fstream::out);
    stan::callbacks::stream_writer sample_writer(output_stream, "# ");

    std::fstream diagnostic_stream(shared_options.diagnostic_file.c_str(),
				   std::fstream::out);
    stan::callbacks::stream_writer diagnostic_writer(diagnostic_stream, "# ");


    //////////////////////////////////////////////////
    //                Initialize Model              //
    //////////////////////////////////////////////////

    std::shared_ptr<stan::io::var_context> var_context
      = get_var_context(shared_options.data_file);

    stan::model::model_base &model
      = new_model(*var_context, shared_options.seed, &std::cout);

    write_stan(sample_writer);
    write_model(sample_writer, model.model_name());
    print_old_command_header(app, shared_options, variational_options, sample_writer);
    write_parallel_info(sample_writer);
    write_opencl_device(sample_writer);

    write_stan(diagnostic_writer);
    write_model(diagnostic_writer, model.model_name());
    print_old_command_header(app, shared_options, variational_options, diagnostic_writer);

    std::shared_ptr<stan::io::var_context> init_context = get_var_context(shared_options.init_file);

    if (variational_options.algorithm
	== VariationalOptions::Algorithm::meanfield) {
      return stan::services::experimental::advi
	::meanfield(model, *init_context,
		    shared_options.seed, shared_options.id,
		    shared_options.init_radius,
		    variational_options.grad_samples,
		    variational_options.elbo_samples,
		    variational_options.iter,
		    variational_options.tol_rel_obj,
		    variational_options.eta,
		    !variational_options.adapt_off,
		    variational_options.adapt_iter,
		    variational_options.eval_elbo,
		    variational_options.output_draws,
		    interrupt, logger,
		    init_writer, sample_writer,
		    diagnostic_writer);
    }
    if (variational_options.algorithm
	== VariationalOptions::Algorithm::fullrank) {
      return stan::services::experimental::advi
	::fullrank(model, *init_context,
		    shared_options.seed, shared_options.id,
		    shared_options.init_radius,
		    variational_options.grad_samples,
		    variational_options.elbo_samples,
		    variational_options.iter,
		    variational_options.tol_rel_obj,
		    variational_options.eta,
		    !variational_options.adapt_off,
		    variational_options.adapt_iter,
		    variational_options.eval_elbo,
		    variational_options.output_draws,
		    interrupt, logger,
		   init_writer, sample_writer,
		   diagnostic_writer);
    }
    return stan::services::error_codes::CONFIG;
  }
}  // namespace cmdstan
#endif
