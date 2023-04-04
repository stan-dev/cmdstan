#ifndef CMDSTAN_ARGUMENTS_ARG_SINGLE_INT_POS_HPP
#define CMDSTAN_ARGUMENTS_ARG_SINGLE_INT_POS_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

/** Generic positive int value argument */

namespace cmdstan {

class arg_single_int_pos : public int_argument {
 public:
  arg_single_int_pos(const char* name, const char* desc, int def)
      : int_argument() {
    _name = name;
    _description = desc;
    _validity = std::string("0 < ").append(name);
    _default = boost::lexical_cast<std::string>(def);
    _default_value = def;
    _constrained = true;
    _good_value = 2.0;
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(int value) { return value > 0; }
};

}  // namespace cmdstan
#endif
