#ifndef CMDSTAN_ARGUMENTS_ARG_ADAPT_WINDOW_HPP
#define CMDSTAN_ARGUMENTS_ARG_ADAPT_WINDOW_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

  class arg_adapt_window: public u_int_argument {
  public:
    arg_adapt_window(): u_int_argument() {
      _name = "window";
      _description = "Initial width of slow adaptation interval";
      _default = "25";
      _default_value = 25;
      _value = _default_value;
    }
  };

}
#endif
