#ifndef CMDSTAN_COMMAND_OPTIMIZE_HPP
#define CMDSTAN_COMMAND_OPTIMIZE_HPP

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
#include <stan/services/optimize/bfgs.hpp>
#include <stan/services/optimize/lbfgs.hpp>
#include <stan/services/optimize/newton.hpp>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace cmdstan {

  int optimize(CLI::App& app,
	       SharedOptions& shared_options,
	       OptimizeOptions& optimize_options) {
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
    print_old_command_header(app, shared_options, optimize_options, sample_writer);
    write_parallel_info(sample_writer);
    write_opencl_device(sample_writer);

    write_stan(diagnostic_writer);
    write_model(diagnostic_writer, model.model_name());
    print_old_command_header(app, shared_options, optimize_options, diagnostic_writer);

    std::shared_ptr<stan::io::var_context> init_context = get_var_context(shared_options.init_file);

    if (optimize_options.algorithm == OptimizeOptions::Algorithm::lbfgs) {
      return stan::services::optimize
	::lbfgs(model, *init_context,
		shared_options.seed, shared_options.id,
		shared_options.init_radius,
		optimize_options.history_size,
		optimize_options.init_alpha,
		optimize_options.tol_obj,
		optimize_options.tol_rel_obj,
		optimize_options.tol_grad,
		optimize_options.tol_rel_grad,
		optimize_options.tol_param,
		optimize_options.iter,
		optimize_options.save_iterations,
		shared_options.refresh, interrupt, logger,
		init_writer, sample_writer);
    }
    if (optimize_options.algorithm == OptimizeOptions::Algorithm::bfgs) {
      return stan::services::optimize
	::bfgs(model, *init_context,
	       shared_options.seed, shared_options.id,
	       shared_options.init_radius,
	       optimize_options.init_alpha,
	       optimize_options.tol_obj,
	       optimize_options.tol_rel_obj,
	       optimize_options.tol_grad,
	       optimize_options.tol_rel_grad,
	       optimize_options.tol_param,
	       optimize_options.iter,
	       optimize_options.save_iterations,
	       shared_options.refresh, interrupt, logger,
	       init_writer, sample_writer);
    }
    if (optimize_options.algorithm == OptimizeOptions::Algorithm::newton) {
      return stan::services::optimize
	::newton(model, *init_context,
		 shared_options.seed, shared_options.id,
		 shared_options.init_radius,
		 optimize_options.iter,
		 optimize_options.save_iterations,
		 interrupt, logger,
		 init_writer, sample_writer);
    }
    return stan::services::error_codes::CONFIG;
  }
}  // namespace cmdstan
#endif
