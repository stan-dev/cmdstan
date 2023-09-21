#ifndef CMDSTAN_WRITE_COMPILE_INFO_HPP
#define CMDSTAN_WRITE_COMPILE_INFO_HPP

#include <stan/callbacks/writer.hpp>
#include <stan/callbacks/structured_writer.hpp>
#include <stan/model/model_base.hpp>

namespace cmdstan {
void write_compile_info(stan::callbacks::writer& writer,
                        stan::model::model_base& model) {
  auto compile_info = model.model_compile_info();
  for (auto s : compile_info) {
    writer(s);
  }
}

void write_compile_info(stan::callbacks::structured_writer& writer,
                        stan::model::model_base& model) {
  auto compile_info = model.model_compile_info();
  for (auto s : compile_info) {
    // split on "="
    std::string::size_type pos = s.find(" = ");
    if (pos == std::string::npos) {
      continue;
    }
    std::string key = s.substr(0, pos);
    std::string value = s.substr(pos + 3);
    writer.write(key, value);
  }
}

}  // namespace cmdstan
#endif
