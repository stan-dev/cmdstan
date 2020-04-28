#ifndef CMDSTAN_WRITE_PARALLEL_INFO_HPP
#define CMDSTAN_WRITE_PARALLEL_INFO_HPP

#include <stan/callbacks/writer.hpp>
#include <stan/math/prim/core/init_threadpool_tbb.hpp>
#include <string>

namespace cmdstan {

void write_parallel_info(stan::callbacks::writer &writer) {
#ifdef STAN_MPI
  writer("mpi_enabled = 1");
#else
#ifdef STAN_THREADS
  std::stringstream msg_threads;
  msg_threads << "num_threads = ";
  msg_threads << stan::math::internal::get_num_threads();
  writer(msg_threads.str());
#endif
#endif
}

}  // namespace cmdstan
#endif
