#ifndef CMDSTAN_ARGUMENTS_ARG_TOLERANCE_HPP
#define CMDSTAN_ARGUMENTS_ARG_TOLERANCE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

namespace cmdstan {

class arg_tolerance : public real_argument {
 public:
  arg_tolerance(const char *name, const char *desc, double def)
      : real_argument() {
    _name = name;
    _description = desc;
    _validity = "0 <= tol";
    _default = boost::lexical_cast<std::string>(def);
    _default_value = def;
    _constrained = true;
    _good_value = 1.0;
    _bad_value = -1.0;
    _value = _default_value;
  }

  bool is_valid(double value) { return value >= 0; }
};

}  // namespace cmdstan
#endif
