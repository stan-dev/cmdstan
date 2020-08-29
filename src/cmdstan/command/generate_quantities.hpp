#ifndef CMDSTAN_COMMAND_GENERATE_QUANTITIES_HPP
#define CMDSTAN_COMMAND_GENERATE_QUANTITIES_HPP

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
#include <stan/services/sample/standalone_gqs.hpp>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace cmdstan {

  int generate_quantities(CLI::App& app,
			  SharedOptions& shared_options,
			  GenerateQuantitiesOptions& gq_options) {
    static int hmc_fixed_cols = 7;  // hmc sampler outputs columns __lp + 6

    stan::callbacks::stream_writer info(std::cout);
    stan::callbacks::stream_writer err(std::cout);
    stan::callbacks::stream_logger logger(std::cout, std::cout, std::cout,
					  std::cerr, std::cerr);

    // Read arguments
    write_parallel_info(info);
    write_opencl_device(info);
    info();

    if (gq_options.fitted_params == shared_options.output_file) {
      std::stringstream msg;
      msg << "Filename conflict, fitted_params file "
	  << gq_options.fitted_params
          << " and output file have same name, must be different."
	  << std::endl;
      throw std::invalid_argument(msg.str());
    }

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
    print_old_command_header(app, shared_options, gq_options, sample_writer);
    write_parallel_info(sample_writer);
    write_opencl_device(sample_writer);

    std::ifstream stream(gq_options.fitted_params.c_str());
    if (stream.rdstate() & std::ifstream::failbit) {
      std::stringstream msg;
      msg << "Can't open specified file, \""
	  << gq_options.fitted_params << "\""
	  << std::endl;
      throw std::invalid_argument(msg.str());
    }

    stan::io::stan_csv fitted_params;
    std::stringstream msg;
    stan::io::stan_csv_reader
      ::read_metadata(stream, fitted_params.metadata, &msg);
    if (!stan::io::stan_csv_reader
	::read_header(stream, fitted_params.header,
		      &msg, false)) {
      msg << "Error reading fitted param names from sample csv file \""
	  << gq_options.fitted_params << "\"" << std::endl;
      throw std::invalid_argument(msg.str());
    }
    stan::io::stan_csv_reader
      ::read_adaptation(stream, fitted_params.adaptation, &msg);
    fitted_params.timing.warmup = 0;
    fitted_params.timing.sampling = 0;
    stan::io::stan_csv_reader::read_samples(stream,
					    fitted_params.samples,
                                            fitted_params.timing, &msg);
    stream.close();

    std::vector<std::string> param_names;
    model.constrained_param_names(param_names, false, false);
    size_t num_cols = param_names.size();
    size_t num_rows = fitted_params.metadata.num_samples;
    // check that all parameter names are in sample, in order
    if (num_cols + hmc_fixed_cols > fitted_params.header.size()) {
      std::stringstream msg;
      msg << "Mismatch between model and fitted_parameters csv file \""
	  << gq_options.fitted_params << "\"" << std::endl;
      throw std::invalid_argument(msg.str());
    }
    for (size_t i = 0; i < num_cols; ++i) {
      if (param_names[i].compare(fitted_params.header[i + hmc_fixed_cols])
          != 0) {
        std::stringstream msg;
        msg << "Mismatch between model and fitted_parameters csv file \""
            << gq_options.fitted_params << "\"" << std::endl;
        throw std::invalid_argument(msg.str());
      }
    }
    return stan::services
      ::standalone_generate(model,
			    fitted_params.samples.block(0, hmc_fixed_cols, num_rows, num_cols),
			    shared_options.seed, interrupt, logger, sample_writer);
    return stan::services::error_codes::CONFIG;
  }
}  // namespace cmdstan
#endif
