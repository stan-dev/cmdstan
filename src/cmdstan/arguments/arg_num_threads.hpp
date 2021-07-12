#ifndef CMDSTAN_ARGUMENTS_ARG_NUM_THREADS_HPP
#define CMDSTAN_ARGUMENTS_ARG_NUM_THREADS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

class arg_num_threads : public int_argument {
 public:
  arg_num_threads() : int_argument() {
    _name = "num_threads";
    _description = std::string("Number of threads available to the program.");
    _default = "1";
    _default_value = 1;
    _value = _default_value;
  }

  bool is_valid(unsigned int value) { return value > -2; }
};

}  // namespace cmdstan
#endif
