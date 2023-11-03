#ifndef CMDSTAN_ARGUMENTS_ARG_SINGLE_REAL_BOUNDED_HPP
#define CMDSTAN_ARGUMENTS_ARG_SINGLE_REAL_BOUNDED_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <string>
#include <sstream>

/** Generic bounded real value argument */

namespace cmdstan {

class arg_single_real_bounded : public real_argument {
  double _lb;
  double _ub;

 public:
  arg_single_real_bounded(const char* name, const char* desc, double def,
                          double lb, double ub)
      : real_argument() {
    _name = name;
    _description = desc;
    _validity
        = std::to_string(lb).append(" <= ").append(name).append(" <= ").append(
            std::to_string(ub));

    std::stringstream def_ss;
    def_ss << def;
    _default = def_ss.str();
    _default_value = def;
    _value = _default_value;
    _lb = lb;
    _ub = ub;
  }

  bool is_valid(double value) { return _lb <= value && value <= _ub; }
};

}  // namespace cmdstan
#endif
