#ifdef STAN_MPI

#include <test/test-models/proper.hpp>
#include <cmdstan/command.hpp>
#include <stan/math/prim.hpp>
#include <gtest/gtest.h>

TEST(StanUiCommand, mpi_ready) {
  // The initialization has all worked if we get to here on the rank=0
  // process.
  EXPECT_TRUE(stan::math::mpi_cluster::listening_status());
}

struct mpi_hello {
  static void distributed_apply() {
    boost::mpi::communicator world;
    boost::mpi::gather(world, world.rank(), 0);
  }
};

// register worker command
STAN_REGISTER_MPI_DISTRIBUTED_APPLY(mpi_hello)

TEST(StanUiCommand, mpi_comm) {
  // perform simple check if a very basic mpi gather works over the
  // stan math mpi building blocks
  std::unique_lock<std::mutex> cluster_lock;
  EXPECT_NO_THROW((cluster_lock = stan::math::mpi_broadcast_command<
                       stan::math::mpi_distributed_apply<mpi_hello>>()));

  EXPECT_TRUE(cluster_lock.owns_lock());

  boost::mpi::communicator world;

  std::vector<int> world_ranks(world.size(), -1);
  boost::mpi::gather(world, world.rank(), world_ranks, 0);

  for (int i = 0; i < world.size(); ++i)
    EXPECT_EQ(world_ranks[i], i);
}

#endif
