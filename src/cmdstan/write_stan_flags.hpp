#ifndef CMDSTAN_WRITE_STAN_FLAGS_HPP
#define CMDSTAN_WRITE_STAN_FLAGS_HPP

#include <stan/callbacks/writer.hpp>
#include <string>

namespace cmdstan {

void write_stan_flags(stan::callbacks::writer &writer) {
#ifdef STAN_THREADS
  writer("STAN_THREADS=true");
#else
  writer("STAN_THREADS=false");
#endif
#ifdef STAN_MPI
  writer("STAN_MPI=true");
#else
  writer("STAN_MPI=false");
#endif
#ifdef STAN_OPENCL
  writer("STAN_OPENCL=true");
#else
  writer("STAN_OPENCL=false");
#endif
#ifdef STAN_NO_RANGE_CHECKS
  writer("STAN_NO_RANGE_CHECKS=true");
#else
  writer("STAN_NO_RANGE_CHECKS=false");
#endif
#ifdef STAN_CPP_OPTIMS
  writer("STAN_CPP_OPTIMS=true");
#else
  writer("STAN_CPP_OPTIMS=false");
#endif
}

}  // namespace cmdstan
#endif
