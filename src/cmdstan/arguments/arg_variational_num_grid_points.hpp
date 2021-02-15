#ifndef CMDSTAN_RVI_ARG_VARIATIONAL_NUM_GRID_POINTS_HPP
#define CMDSTAN_RVI_ARG_VARIATIONAL_NUM_GRID_POINTS_HPP
#include <cmdstan/arguments/singleton_argument.hpp>
num_grid_points
namespace cmdstan {

    class arg_variational_num_grid_points : public real_argument {
    public:
        arg_variational_num_grid_points() : real_argument() {
          _name = "num_grid_pointsy";
          _description = "Number of iterate values to calculate min(Rhat)";
          _validity = "0.0 < num_grid_points";
          _default = "5";
          _default_value = 5;
          _constrained = true;
          _good_value = 5;
          _bad_value = -1.0;
          _value = _default_value;
        }

        bool is_valid(int value) { return value > 0.0; }
    };

}
#endif//CMDSTAN_RVI_ARG_VARIATIONAL_NUM_GRID_POINTS_HPP
