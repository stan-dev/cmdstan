#ifndef CMDSTAN_WRITE_HEADER_HPP
#define CMDSTAN_WRITE_HEADER_HPP

#include <stan/callbacks/writer.hpp>
#include <string>
#include <vector>

namespace cmdstan {
  void write_header(stan::callbacks::writer& writer,
                   const std::vector<std::string> names) {
    writer(names);
  }
}
#endif
