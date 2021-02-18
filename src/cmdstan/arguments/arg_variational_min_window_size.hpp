#ifndef CMDSTAN_RVI_ARG_VARIATIONAL_MIN_WINDOW_SIZE_HPP
#define CMDSTAN_RVI_ARG_VARIATIONAL_MIN_WINDOW_SIZE_HPP

#include <cmdstan/arguments/singleton_argument.hpp>

namespace cmdstan {

    class arg_variational_min_window_size : public int_argument {
    public:
        arg_variational_min_window_size() : int_argument() {
          _name = "min_window_size";
          _description = "Minimum size of window to produce optimal window size"
                         "related to check_frequcy and ess_cut";
          _validity = "0.0 < min_window_size";
          _default = "200";
          _default_value = 200;
          _constrained = true;
          _good_value = 200;
          _bad_value = -1.0;
          _value = _default_value;
        }

        bool is_valid(int value) { return value > 0.0; }
    };

}
#endif //CMDSTAN_RVI_ARG_VARIATIONAL_MIN_WINDOW_SIZE_H
