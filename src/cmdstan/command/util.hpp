#ifndef CMDSTAN_COMMAND_UTIL_HPP
#define CMDSTAN_COMMAND_UTIL_HPP

#include <stan/model/model_base.hpp>
#include <stan/io/dump.hpp>
#include <stan/io/ends_with.hpp>
#include <stan/io/var_context.hpp>
#include <cmdstan/io/json/json_data.hpp>
#include <iostream>

// forward declaration for function defined in another translation unit
stan::model::model_base &new_model(stan::io::var_context &data_context,
                                   unsigned int seed, std::ostream *msg_stream);

namespace cmdstan {

  std::shared_ptr<stan::io::var_context> get_var_context(const std::string file) {
    std::fstream stream(file.c_str(), std::fstream::in);
    if (file != "" && (stream.rdstate() & std::ifstream::failbit)) {
      std::stringstream msg;
      msg << "Can't open specified file, \"" << file << "\"" << std::endl;
      throw std::invalid_argument(msg.str());
    }
    if (stan::io::ends_with(".json", file)) {
      cmdstan::json::json_data var_context(stream);
      stream.close();
      std::shared_ptr<stan::io::var_context> result
        = std::make_shared<cmdstan::json::json_data>(var_context);
      return result;
    }
    stan::io::dump var_context(stream);
    stream.close();
    std::shared_ptr<stan::io::var_context> result
      = std::make_shared<stan::io::dump>(var_context);
    return result;
  }

}  // namespace cmdstan

#endif
