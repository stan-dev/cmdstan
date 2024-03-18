#ifndef CMDSTAN_WRITE_STAN_HPP
#define CMDSTAN_WRITE_STAN_HPP

#include <stan/callbacks/writer.hpp>
#include <stan/callbacks/structured_writer.hpp>
#include <stan/version.hpp>
#include <string>

namespace cmdstan {

void write_stan(stan::callbacks::writer &writer) {
  writer("stan_version_major = " + stan::MAJOR_VERSION);
  writer("stan_version_minor = " + stan::MINOR_VERSION);
  writer("stan_version_patch = " + stan::PATCH_VERSION);
}

void write_stan(stan::callbacks::structured_writer &writer) {
  writer.write("stan_major_version", stan::MAJOR_VERSION);
  writer.write("stan_minor_version", stan::MINOR_VERSION);
  writer.write("stan_patch_version", stan::PATCH_VERSION);
}

}  // namespace cmdstan
#endif
