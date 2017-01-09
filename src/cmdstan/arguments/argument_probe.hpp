#ifndef CMDSTAN_ARGUMENTS_ARGUMENT_PROBE_HPP
#define CMDSTAN_ARGUMENTS_ARGUMENT_PROBE_HPP

#include <stan/callbacks/writer.hpp>
#include <cmdstan/arguments/argument.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace cmdstan {

  class argument_probe {
  public:
    explicit argument_probe(std::vector<argument*>& valid_args)
      : _arguments(valid_args) {}

    void probe_args(stan::callbacks::writer& w) {
      for (std::vector<argument*>::iterator arg_it = _arguments.begin();
           arg_it != _arguments.end(); ++arg_it)
        (*arg_it)->probe_args(*arg_it, w);
    }

  protected:
    std::vector<argument*>& _arguments;
  };

}
#endif
