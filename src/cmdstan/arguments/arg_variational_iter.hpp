#ifndef STAN_VARIATIONAL_ITER_HPP
#define STAN_VARIATIONAL_ITER_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <stan/services/experimental/advi/defaults.hpp>
#include <boost/lexical_cast.hpp>

namespace cmdstan {

  using stan::services::experimental::advi::max_iterations;
  
  class arg_variational_iter: public int_argument {
  public:
    arg_variational_iter(): int_argument() {
      _name = "iter";
      _description = max_iterations::description();
      _validity = "0 < iter";
      _default = boost::lexical_cast<std::string>(max_iterations::default_value());
      _default_value = max_iterations::default_value();
      _constrained = true;
      _good_value = max_iterations::default_value();
      _bad_value = -1.0;
      _value = _default_value;
    }

    bool is_valid(int value) {
      return value > 0;
    }
  };

}
#endif
