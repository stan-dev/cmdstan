#ifndef CMDSTAN_ARGUMENTS_ARG_VARIATIONAL_ADAPT_ENGAGED_HPP
#define CMDSTAN_ARGUMENTS_ARG_VARIATIONAL_ADAPT_ENGAGED_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#include <stan/services/experimental/advi/defaults.hpp>
#include <boost/lexical_cast.hpp>

namespace cmdstan {

  using stan::services::experimental::advi::adapt_engaged;
  
  class arg_variational_adapt_engaged: public bool_argument {
  public:
    arg_variational_adapt_engaged(): bool_argument() {
      _name = "engaged";
      _description = adapt_engaged::description();
      _validity = "[0, 1]";
      _default = boost::lexical_cast<std::string>(adapt_engaged::default_value());
      _default_value = adapt_engaged::default_value();
      _constrained = false;
      _good_value = adapt_engaged::default_value();
      _value = _default_value;
    }
  };

}
#endif
