#ifndef CMDSTAN_ARGUMENTS_ARG_ADAPT_NUM_CROSS_CHAIN_HPP
#define CMDSTAN_ARGUMENTS_ARG_ADAPT_NUM_CROSS_CHAIN_HPP

#include <cmdstan/arguments/singleton_argument.hpp>
#ifdef MPI_ADAPTED_WARMUP
#include <stan/math/mpi/envionment.hpp>
#endif

namespace cmdstan {

  class arg_adapt_num_cross_chains: public u_int_argument {
  public:
    arg_adapt_num_cross_chains(): u_int_argument() {
      _name = "num_cross_chains";
      _description = "Number of chains in cross-chain warmup iterations";
#ifdef MPI_ADAPTED_WARMUP
      _validity = "num_cross_chains <= number of MPI processes";
      _default = "number of MPI processes";
      int stan_world_size;
      MPI_Comm_size(MPI_COMM_STAN, &stan_world_size);
      _default_value = stan_world_size;
      _constrained = true;
      _good_value = 2;
      _bad_value = -1;
#else
      _validity = "0 < num_cross_chains";
      _default = "1";
      _default_value = 1;
#endif
      _value = _default_value;
    }

#ifdef MPI_ADAPTED_WARMUP
    bool is_valid(unsigned int value) {
      int stan_world_size;
      MPI_Comm_size(MPI_COMM_STAN, &stan_world_size);
   return value <= stan_world_size;
    }
#endif
  };

}
#endif
