#ifndef CMDSTAN_WRITE_PARALLEL_INFO_HPP
#define CMDSTAN_WRITE_PARALLEL_INFO_HPP

#include <stan/callbacks/writer.hpp>
#include <stan/math/prim/core/init_threadpool_tbb.hpp>
#include <string>

namespace cmdstan {

void write_parallel_info(stan::callbacks::writer &writer) {
#ifdef STAN_MPI
  writer("mpi_enabled = 1");
#endif
}

}  // namespace cmdstan
#endif
