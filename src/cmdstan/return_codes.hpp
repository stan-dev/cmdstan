#ifndef CMDSTAN_RETURN_CODES_HPP
#define CMDSTAN_RETURN_CODES_HPP

namespace cmdstan {

struct return_codes {
  enum { OK = 0, NOT_OK = 1 };
};

}  // namespace cmdstan
#endif
