#ifndef CMDSTAN_WRITE_STAN_HPP
#define CMDSTAN_WRITE_STAN_HPP

#include <stan/callbacks/writer.hpp>
#include <stan/version.hpp>
#include <string>

namespace cmdstan {

  void write_stan(stan::callbacks::writer& writer) {
    writer("stan_version_major = " + stan::MAJOR_VERSION);
    writer("stan_version_minor = " + stan::MINOR_VERSION);
    writer("stan_version_patch = " + stan::PATCH_VERSION);
  }

}
#endif
