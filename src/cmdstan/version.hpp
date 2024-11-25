#ifndef CMDSTAN_VERSION_HPP
#define CMDSTAN_VERSION_HPP

#include <string>

#ifndef STAN_STRING_EXPAND
#define STAN_STRING_EXPAND(s) #s
#endif

#ifndef STAN_STRING
#define STAN_STRING(s) STAN_STRING_EXPAND(s)
#endif

#define CMDSTAN_MAJOR 2
#define CMDSTAN_MINOR 36
#define CMDSTAN_PATCH 0

namespace cmdstan {

/** Major version number for CmdStan package. */
const std::string MAJOR_VERSION = STAN_STRING(CMDSTAN_MAJOR);

/** Minor version number for CmdStan package. */
const std::string MINOR_VERSION = STAN_STRING(CMDSTAN_MINOR);

/** Patch version for CmdStan package. */
const std::string PATCH_VERSION = STAN_STRING(CMDSTAN_PATCH);

}  // namespace cmdstan

#endif
