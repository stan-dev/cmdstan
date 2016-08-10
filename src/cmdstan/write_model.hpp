#ifndef CMDSTAN_WRITE_MODEL_HPP
#define CMDSTAN_WRITE_MODEL_HPP

#include <stan/callbacks/writer.hpp>
#include <string>

namespace cmdstan {
  void write_model(stan::callbacks::writer& writer,
                   const std::string model_name) {
    writer("model = " + model_name);
  }
}
#endif
