#ifndef CMDSTAN_RVI_ARG_VARIATIONAL_CHECK_FREQUENCY_HPP
#define CMDSTAN_RVI_ARG_VARIATIONAL_CHECK_FREQUENCY_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

    class arg_variational_check_frequency : public int_argument {
    public:
        arg_variational_check_frequency() : int_argument() {
          _name = "check_frequency";
          _description = "How often samples are checked for convergence";
          _validity = "0 < check_frequency";
          _default = "25";
          _default_value = 25;
          _constrained = true;
          _good_value = 25;
          _bad_value = -1.0;
          _value = _default_value;
        }

        bool is_valid(int value) { return value > 0; }
    };

}
#endif //CMDSTAN_RVI_ARG_VARIATIONAL_CHECK_FREQUENCY_HPP
