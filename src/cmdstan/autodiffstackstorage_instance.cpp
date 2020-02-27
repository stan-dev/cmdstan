#include <stan/math/rev/core.hpp>

namespace stan {
  namespace math {

    template <>
    STAN_THREADS_DEF
    typename AutodiffStackSingleton<vari, chainable_alloc>::AutodiffStackStorage*
    AutodiffStackSingleton<vari, chainable_alloc>::instance_ = nullptr;
  }
}
