#ifndef CMDSTAN_ARGUMENTS_VARIATIONAL_ETA_HPP
#define CMDSTAN_ARGUMENTS_VARIATIONAL_ETA_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <stan/services/experimental/advi/defaults.hpp>
#include <boost/lexical_cast.hpp>

namespace cmdstan {

  using stan::services::experimental::advi::eta;
  
  class arg_variational_eta: public real_argument {
  public:
    arg_variational_eta(): real_argument() {
      _name = "eta";
      _description = eta::description();
      _validity = "0 < eta";
      _default = boost::lexical_cast<std::string>(eta::default_value());
      _default_value = eta::default_value();
      _constrained = true;
      _good_value = eta::default_value();
      _bad_value = -1.0;
      _value = _default_value;
    }
    bool is_valid(double value) { return value > 0; }
  };
}
#endif
