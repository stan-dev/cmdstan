#ifndef CMDSTAN_COMMAND_SAMPLE_HPP
#define CMDSTAN_COMMAND_SAMPLE_HPP

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
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace cmdstan {

  int sample(CLI::App& app,
	     SharedOptions& shared_options,
	     SampleOptions& sample_options) {
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
    print_old_command_header(app, shared_options, sample_options, sample_writer);
    write_parallel_info(sample_writer);
    write_opencl_device(sample_writer);

    write_stan(diagnostic_writer);
    write_model(diagnostic_writer, model.model_name());
    print_old_command_header(app, shared_options, sample_options, diagnostic_writer);

    std::shared_ptr<stan::io::var_context> init_context = get_var_context(shared_options.init_file);
    std::shared_ptr<stan::io::var_context> metric_context = get_var_context(sample_options.metric_file);

    if (!sample_options.adapt_off && sample_options.num_warmup == 0) {
      info("The number of warmup samples (num_warmup) must be greater than "
	   "zero if adaptation is enabled.");
      return stan::services::error_codes::CONFIG;
    }

    if (model.num_params_r() == 0
	&& sample_options.algorithm != SampleOptions::Algorithm::fixed_param) {
      info("Must use --fixed_param for model that has no parameters.");
      return stan::services::error_codes::CONFIG;
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::nuts
	&& sample_options.metric == SampleOptions::Metric::dense
	&& sample_options.adapt_off) {
      if (sample_options.metric_file == "") {
	return stan::services::sample
	  ::hmc_nuts_dense_e(model, *init_context,
			     shared_options.seed, shared_options.id,
			     shared_options.init_radius, sample_options.num_warmup,
			     sample_options.num_samples, sample_options.thin,
			     sample_options.save_warmup,
			     shared_options.refresh, sample_options.stepsize,
			     sample_options.stepsize_jitter, sample_options.max_depth,
			     interrupt, logger, init_writer,
			     sample_writer, diagnostic_writer);
      } else {
	return stan::services::sample
	  ::hmc_nuts_dense_e(model, *init_context, *metric_context,
			     shared_options.seed, shared_options.id,
			     shared_options.init_radius, sample_options.num_warmup,
			     sample_options.num_samples, sample_options.thin,
			     sample_options.save_warmup,
			     shared_options.refresh, sample_options.stepsize,
			     sample_options.stepsize_jitter, sample_options.max_depth,
			     interrupt, logger, init_writer,
			     sample_writer, diagnostic_writer);
      }
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::nuts
	&& sample_options.metric == SampleOptions::Metric::dense
	&& !sample_options.adapt_off) {
      if (sample_options.metric_file == "") {
	return stan::services::sample
	  ::hmc_nuts_dense_e_adapt(model, *init_context,
				   shared_options.seed, shared_options.id,
				   shared_options.init_radius, sample_options.num_warmup,
				   sample_options.num_samples, sample_options.thin,
				   sample_options.save_warmup,
				   shared_options.refresh, sample_options.stepsize,
				   sample_options.stepsize_jitter, sample_options.max_depth,
				   sample_options.delta, sample_options.gamma, sample_options.kappa,
				   sample_options.t0, sample_options.init_buffer,
				   sample_options.term_buffer, sample_options.window,
				   interrupt, logger, init_writer,
				   sample_writer, diagnostic_writer);
      } else {
	return stan::services::sample
	  ::hmc_nuts_dense_e_adapt(model, *init_context, *metric_context,
				   shared_options.seed, shared_options.id,
				   shared_options.init_radius, sample_options.num_warmup,
				   sample_options.num_samples, sample_options.thin,
				   sample_options.save_warmup,
				   shared_options.refresh, sample_options.stepsize,
				   sample_options.stepsize_jitter, sample_options.max_depth,
				   sample_options.delta, sample_options.gamma, sample_options.kappa,
				   sample_options.t0, sample_options.init_buffer,
				   sample_options.term_buffer, sample_options.window,
				   interrupt, logger, init_writer,
				   sample_writer, diagnostic_writer);
      }
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::nuts
	&& sample_options.metric == SampleOptions::Metric::diag
	&& sample_options.adapt_off) {
      if (sample_options.metric_file == "") {
	return stan::services::sample
	  ::hmc_nuts_diag_e(model, *init_context,
			    shared_options.seed, shared_options.id,
			    shared_options.init_radius, sample_options.num_warmup,
			    sample_options.num_samples, sample_options.thin,
			    sample_options.save_warmup, shared_options.refresh,
			    sample_options.stepsize, sample_options.stepsize_jitter,
			    sample_options.max_depth,
			    interrupt, logger, init_writer,
			    sample_writer, diagnostic_writer);

      } else {
	return stan::services::sample
	  ::hmc_nuts_diag_e(model, *init_context, *metric_context,
			    shared_options.seed, shared_options.id,
			    shared_options.init_radius, sample_options.num_warmup,
			    sample_options.num_samples, sample_options.thin,
			    sample_options.save_warmup, shared_options.refresh,
			    sample_options.stepsize, sample_options.stepsize_jitter,
			    sample_options.max_depth,
			    interrupt, logger, init_writer,
			    sample_writer, diagnostic_writer);
      }
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::nuts
	&& sample_options.metric == SampleOptions::Metric::diag
	&& !sample_options.adapt_off) {
      if (sample_options.metric_file == "") {
	return stan::services::sample
	  ::hmc_nuts_diag_e_adapt(model, *init_context,
				  shared_options.seed, shared_options.id,
				  shared_options.init_radius, sample_options.num_warmup,
				  sample_options.num_samples, sample_options.thin,
				  sample_options.save_warmup, shared_options.refresh,
				  sample_options.stepsize,
				  sample_options.stepsize_jitter, sample_options.max_depth,
				  sample_options.delta, sample_options.gamma, sample_options.kappa,
				  sample_options.t0, sample_options.init_buffer,
				  sample_options.term_buffer, sample_options.window,
				  interrupt, logger, init_writer,
				  sample_writer, diagnostic_writer);

      } else {
	return stan::services::sample
	  ::hmc_nuts_diag_e_adapt(model, *init_context, *metric_context,
				  shared_options.seed, shared_options.id,
				  shared_options.init_radius, sample_options.num_warmup,
				  sample_options.num_samples, sample_options.thin,
				  sample_options.save_warmup, shared_options.refresh,
				  sample_options.stepsize, sample_options.stepsize_jitter,
				  sample_options.max_depth,
				  sample_options.delta, sample_options.gamma, sample_options.kappa,
				  sample_options.t0, sample_options.init_buffer,
				  sample_options.term_buffer, sample_options.window,
				  interrupt, logger, init_writer,
				  sample_writer, diagnostic_writer);
      }
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::nuts
	&& sample_options.metric == SampleOptions::Metric::unit
	&& sample_options.adapt_off) {
      return stan::services::sample
	::hmc_nuts_unit_e(model, *init_context,
			  shared_options.seed, shared_options.id,
			  shared_options.init_radius, sample_options.num_warmup,
			  sample_options.num_samples, sample_options.thin,
			  sample_options.save_warmup, shared_options.refresh,
			  sample_options.stepsize, sample_options.stepsize_jitter,
			  sample_options.max_depth,
			  interrupt, logger, init_writer,
			  sample_writer, diagnostic_writer);
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::nuts
	&& sample_options.metric == SampleOptions::Metric::unit
	&& !sample_options.adapt_off) {
      return stan::services::sample
	::hmc_nuts_unit_e_adapt(model, *init_context,
				shared_options.seed, shared_options.id,
				shared_options.init_radius, sample_options.num_warmup,
				sample_options.num_samples, sample_options.thin,
				sample_options.save_warmup, shared_options.refresh,
				sample_options.stepsize, sample_options.stepsize_jitter,
				sample_options.max_depth,
				sample_options.delta, sample_options.gamma, sample_options.kappa,
				sample_options.t0,
				interrupt, logger, init_writer,
				sample_writer, diagnostic_writer);
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::hmc
	&& sample_options.metric == SampleOptions::Metric::dense
	&& sample_options.adapt_off) {
      if (sample_options.metric_file == "") {
      	return stan::services::sample
	  ::hmc_static_dense_e(model, *init_context,
			       shared_options.seed, shared_options.id,
			       shared_options.init_radius, sample_options.num_warmup,
			       sample_options.num_samples, sample_options.thin,
			       sample_options.save_warmup,
			       shared_options.refresh, sample_options.stepsize,
			       sample_options.stepsize_jitter, sample_options.int_time,
			       interrupt, logger, init_writer,
			       sample_writer, diagnostic_writer);
      } else {
      	return stan::services::sample
	  ::hmc_static_dense_e(model, *init_context, *metric_context,
			       shared_options.seed, shared_options.id,
			       shared_options.init_radius, sample_options.num_warmup,
			       sample_options.num_samples, sample_options.thin,
			       sample_options.save_warmup,
			       shared_options.refresh, sample_options.stepsize,
			       sample_options.stepsize_jitter, sample_options.int_time,
			       interrupt, logger, init_writer,
			       sample_writer, diagnostic_writer);
      }
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::hmc
	&& sample_options.metric == SampleOptions::Metric::dense
	&& !sample_options.adapt_off) {
      if (sample_options.metric_file == "") {
      	return stan::services::sample
	  ::hmc_static_dense_e_adapt(model, *init_context,
				     shared_options.seed, shared_options.id,
				     shared_options.init_radius, sample_options.num_warmup,
				     sample_options.num_samples, sample_options.thin,
				     sample_options.save_warmup,
				     shared_options.refresh, sample_options.stepsize,
				     sample_options.stepsize_jitter, sample_options.int_time,
				     sample_options.delta, sample_options.gamma, sample_options.kappa,
				     sample_options.t0, sample_options.init_buffer,
				     sample_options.term_buffer, sample_options.window,
				     interrupt, logger, init_writer,
				     sample_writer, diagnostic_writer);
      } else {
      	return stan::services::sample
	  ::hmc_static_dense_e_adapt(model, *init_context, *metric_context,
				     shared_options.seed, shared_options.id,
				     shared_options.init_radius, sample_options.num_warmup,
				     sample_options.num_samples, sample_options.thin,
				     sample_options.save_warmup,
				     shared_options.refresh, sample_options.stepsize,
				     sample_options.stepsize_jitter, sample_options.int_time,
				     sample_options.delta, sample_options.gamma, sample_options.kappa,
				     sample_options.t0, sample_options.init_buffer,
				     sample_options.term_buffer, sample_options.window,
				     interrupt, logger, init_writer,
				     sample_writer, diagnostic_writer);
      }
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::hmc
	&& sample_options.metric == SampleOptions::Metric::diag
	&& sample_options.adapt_off) {
      if (sample_options.metric_file == "") {
      	return stan::services::sample
	  ::hmc_static_diag_e(model, *init_context,
			      shared_options.seed, shared_options.id,
			      shared_options.init_radius, sample_options.num_warmup,
			      sample_options.num_samples, sample_options.thin,
			      sample_options.save_warmup,
			      shared_options.refresh, sample_options.stepsize,
			      sample_options.stepsize_jitter, sample_options.int_time,
			      interrupt, logger, init_writer,
			      sample_writer, diagnostic_writer);
      } else {
      	return stan::services::sample
	  ::hmc_static_diag_e(model, *init_context, *metric_context,
			      shared_options.seed, shared_options.id,
			      shared_options.init_radius, sample_options.num_warmup,
			      sample_options.num_samples, sample_options.thin,
			      sample_options.save_warmup,
			      shared_options.refresh, sample_options.stepsize,
			      sample_options.stepsize_jitter, sample_options.int_time,
			      interrupt, logger, init_writer,
			      sample_writer, diagnostic_writer);
      }
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::hmc
	&& sample_options.metric == SampleOptions::Metric::diag
	&& !sample_options.adapt_off) {
      if (sample_options.metric_file == "") {
      	return stan::services::sample
	  ::hmc_static_diag_e_adapt(model, *init_context,
				    shared_options.seed, shared_options.id,
				    shared_options.init_radius, sample_options.num_warmup,
				    sample_options.num_samples, sample_options.thin,
				    sample_options.save_warmup,
				    shared_options.refresh, sample_options.stepsize,
				    sample_options.stepsize_jitter, sample_options.int_time,
				    sample_options.delta, sample_options.gamma, sample_options.kappa,
				    sample_options.t0, sample_options.init_buffer,
				    sample_options.term_buffer, sample_options.window,
				    interrupt, logger, init_writer,
				    sample_writer, diagnostic_writer);
      } else {
      	return stan::services::sample
	  ::hmc_static_diag_e_adapt(model, *init_context, *metric_context,
				    shared_options.seed, shared_options.id,
				    shared_options.init_radius, sample_options.num_warmup,
				    sample_options.num_samples, sample_options.thin,
				    sample_options.save_warmup,
				    shared_options.refresh, sample_options.stepsize,
				    sample_options.stepsize_jitter, sample_options.int_time,
				    sample_options.delta, sample_options.gamma, sample_options.kappa,
				    sample_options.t0, sample_options.init_buffer,
				    sample_options.term_buffer, sample_options.window,
				    interrupt, logger, init_writer,
				    sample_writer, diagnostic_writer);
      }
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::hmc
	&& sample_options.metric == SampleOptions::Metric::unit
	&& sample_options.adapt_off) {
      return stan::services::sample
	::hmc_static_unit_e(model, *init_context,
			    shared_options.seed, shared_options.id,
			    shared_options.init_radius, sample_options.num_warmup,
			    sample_options.num_samples, sample_options.thin,
			    sample_options.save_warmup, shared_options.refresh,
			    sample_options.stepsize, sample_options.stepsize_jitter,
			    sample_options.int_time,
			    interrupt, logger, init_writer,
			    sample_writer, diagnostic_writer);
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::hmc
	&& sample_options.metric == SampleOptions::Metric::unit
	&& !sample_options.adapt_off) {
      return stan::services::sample
	::hmc_static_unit_e_adapt(model, *init_context,
				  shared_options.seed, shared_options.id,
				  shared_options.init_radius, sample_options.num_warmup,
				  sample_options.num_samples, sample_options.thin,
				  sample_options.save_warmup, shared_options.refresh,
				  sample_options.stepsize, sample_options.stepsize_jitter,
				  sample_options.int_time,
				  sample_options.delta, sample_options.gamma, sample_options.kappa,
				  sample_options.t0,
				  interrupt, logger, init_writer,
				  sample_writer, diagnostic_writer);
    }

    if (sample_options.algorithm == SampleOptions::Algorithm::fixed_param) {
      return stan::services::sample
	::fixed_param(model, *init_context,
		      shared_options.seed, shared_options.id,
		      shared_options.init_radius,
		      sample_options.num_samples, sample_options.thin,
		      shared_options.refresh,
		      interrupt, logger, init_writer,
		      sample_writer, diagnostic_writer);
    }
    return stan::services::error_codes::CONFIG;
  }
}  // namespace cmdstan
#endif
