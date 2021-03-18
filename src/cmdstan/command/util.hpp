#ifndef CMDSTAN_COMMAND_UTIL_HPP
#define CMDSTAN_COMMAND_UTIL_HPP

#include <stan/model/model_base.hpp>
#include <stan/io/dump.hpp>
#include <stan/io/ends_with.hpp>
#include <stan/io/var_context.hpp>
#include <cmdstan/io/json/json_data.hpp>
#include <cmdstan/cli.hpp>
#include <cmdstan/write_profiling.hpp>
#include <iostream>

// forward declaration for function defined in another translation unit
stan::model::model_base &new_model(stan::io::var_context &data_context,
                                   unsigned int seed, std::ostream *msg_stream);
stan::math::profile_map &get_stan_profile_data();

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

void export_profile_info(CLI::App &app, SharedOptions &shared_options) {
  stan::math::profile_map &profile_data = get_stan_profile_data();
  if (profile_data.size() > 0) {
    std::fstream profile_stream(shared_options.profile_file.c_str(),
                                std::fstream::out);
    if (app.get_subcommand()->count("--sig_figs"))
      profile_stream << std::setprecision(shared_options.sig_figs);
    write_profiling(profile_stream, profile_data);
    profile_stream.close();
  }
}

}  // namespace cmdstan

#endif
