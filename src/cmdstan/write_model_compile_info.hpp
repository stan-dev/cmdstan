#ifndef CMDSTAN_WRITE_COMPILE_INFO_HPP
#define CMDSTAN_WRITE_COMPILE_INFO_HPP

#include <stan/callbacks/writer.hpp>
#include <stan/version.hpp>
#include <string>
#include <vector>

namespace cmdstan {
void write_compile_info(stan::callbacks::writer& writer,
                        std::vector<std::string>& compile_info) {
  for (int i = 0; i < compile_info.size(); i++) {
    writer(compile_info[i]);
  }
}
}  // namespace cmdstan
#endif
