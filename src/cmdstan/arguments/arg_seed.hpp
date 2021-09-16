#ifndef CMDSTAN_ARGUMENTS_ARG_SEED_HPP
#define CMDSTAN_ARGUMENTS_ARG_SEED_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace cmdstan {

class arg_seed : public long_long_int_argument {
 public:
  unsigned int _random_value;
  arg_seed() : long_long_int_argument() {
    _name = "seed";
    _description = "Random number generator seed";
    _validity
        = "non-negative integer < 4294967296  or -1 to generate seed from "
          "system time";
    _default = "-1";
    _default_value = -1;
    _constrained = true;
    _good_value = 18383;
    _bad_value = -2;
    _value = _default_value;
    _random_value
        = (boost::posix_time::microsec_clock::universal_time()
           - boost::posix_time::ptime(boost::posix_time::min_date_time))
              .total_milliseconds();
  }

  bool is_valid(long long int value) {
    return (value <= UINT_MAX && value >= 0) || value == _default_value;
  }

  unsigned int random_value() {
    if (_value == _default_value) {
      return _random_value;
    } else {
      return _value;
    }
  }

  std::string print_value() {
    if (_value == _default_value) {
      return boost::lexical_cast<std::string>(_random_value);
    } else {
      return boost::lexical_cast<std::string>(_value);
    }
  }
};

}  // namespace cmdstan
#endif
