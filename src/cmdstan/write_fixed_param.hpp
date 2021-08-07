#ifndef CMDSTAN_WRITE_FIXED_PARAM_HPP
#define CMDSTAN_WRITE_FIXED_PARAM_HPP

#include <stan/callbacks/writer.hpp>
#include <stan/version.hpp>
#include <string>

namespace cmdstan {
void write_fixed_param(stan::callbacks::writer& writer) {
  std::stringstream fixed_param_msg;
  fixed_param_msg << "fixed_param = 1";
  writer(fixed_param_msg.str());
}
}  // namespace cmdstan
#endif
