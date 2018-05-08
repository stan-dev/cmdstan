#include <cmdstan/command.hpp>
#include <stan/services/error_codes.hpp>
#include <boost/exception/diagnostic_information.hpp> 
#include <boost/exception_ptr.hpp>

#ifdef STAN_MPI
#include <stan/math/prim/arr/functor/mpi_cluster.hpp>
#endif

int main(int argc, const char* argv[]) {
  int exitcode = 0;
#ifdef STAN_MPI
  stan::math::mpi_cluster cluster;
  std::cout << "Starting MPI process " << cluster.rank_+1 << " / " << cluster.world_.size() << std::endl;
  cluster.listen();
  if (cluster.rank_ != 0) return 0;
#endif
  try {
    exitcode = cmdstan::command<stan_model>(argc,argv);
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    exitcode = stan::services::error_codes::SOFTWARE;
  }
  return exitcode;
}
