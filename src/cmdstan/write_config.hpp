#ifndef CMDSTAN_WRITE_CONFIG_HPP
#define CMDSTAN_WRITE_CONFIG_HPP

#include <stan/callbacks/writer.hpp>
#include <stan/callbacks/structured_writer.hpp>
#include <stan/model/model_base.hpp>
#include <stan/version.hpp>
#include <cmdstan/arguments/argument_parser.hpp>
#include <cmdstan/write_datetime.hpp>
#include <cmdstan/write_model_compile_info.hpp>
#include <cmdstan/write_model.hpp>
#include <cmdstan/write_opencl_device.hpp>
#include <cmdstan/write_parallel_info.hpp>
#include <cmdstan/write_profiling.hpp>
#include <cmdstan/write_stan.hpp>

namespace cmdstan {

inline void write_config(stan::callbacks::writer &writer,
                         argument_parser &parser,
                         stan::model::model_base &model) {
  write_stan(writer);
  write_model(writer, model.model_name());
  write_datetime(writer);
  parser.print(writer);
  write_parallel_info(writer);
  write_opencl_device(writer);
  write_compile_info(writer, model);
}

inline void write_config(stan::callbacks::structured_writer &writer,
                         argument_parser &parser,
                         stan::model::model_base &model) {
  writer.begin_record();
  write_stan(writer);
  writer.write("model_name", model.model_name());
  writer.write("start_datetime", current_datetime());
  parser.print(writer);
#ifdef STAN_MPI
  writer.write("mpi_enabled", true);
#else
  writer.write("mpi_enabled", false);
#endif
  write_opencl_device(writer);
  write_compile_info(writer, model);
  writer.end_record();
}

}  // namespace cmdstan
#endif
