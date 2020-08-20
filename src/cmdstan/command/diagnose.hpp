#ifndef CMDSTAN_COMMAND_DIAGNOSE_HPP
#define CMDSTAN_COMMAND_DIAGNOSE_HPP

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
#include <stan/services/diagnose/diagnose.hpp>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace cmdstan {

  int diagnose(CLI::App& app,
	       SharedOptions& shared_options,
	       DiagnoseOptions& diagnose_options) {
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

    //////////////////////////////////////////////////
    //                Initialize Model              //
    //////////////////////////////////////////////////

    std::shared_ptr<stan::io::var_context> var_context
      = get_var_context(shared_options.data_file);

    stan::model::model_base &model
      = new_model(*var_context, shared_options.seed, &std::cout);

    write_stan(sample_writer);
    write_model(sample_writer, model.model_name());
    print_old_command_header(app, shared_options, diagnose_options, sample_writer);
    write_parallel_info(sample_writer);
    write_opencl_device(sample_writer);

    std::shared_ptr<stan::io::var_context> init_context = get_var_context(shared_options.init_file);

    return stan::services::diagnose
      ::diagnose(model, *init_context,
		 shared_options.seed,
		 shared_options.id,
		 shared_options.init_radius,
		 diagnose_options.epsilon,
		 diagnose_options.threshold,
		 interrupt, logger, init_writer,
		 sample_writer);
  }
}  // namespace cmdstan
#endif
