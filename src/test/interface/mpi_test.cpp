#ifdef STAN_MPI

#include <cmdstan/command.hpp>
#include <gtest/gtest.h>
#include <stan/math/prim/arr.hpp>

// google test for MPI requires to disable output for non-root
// (rank!=0) processes and adequate initialization of the MPI
// ressource
std::size_t rank;
std::size_t world_size;

class MPIEnvironment : public ::testing::Environment {
 public:
  virtual void SetUp() {
    stan::math::mpi_cluster& cluster = cmdstan::get_mpi_cluster();
    rank = cluster.rank_;
    world_size = cluster.world_.size();
    ::testing::TestEventListeners& listeners
        = ::testing::UnitTest::GetInstance()->listeners();
    if (rank != 0) {
      delete listeners.Release(listeners.default_result_printer());
    }

    cluster.listen();
  }
  virtual void TearDown() {
  }

  virtual ~MPIEnvironment() {}
};

// register MPI global
::testing::Environment* const mpi_env
    = ::testing::AddGlobalTestEnvironment(new MPIEnvironment);

TEST(StanUiCommand, mpi_ready) {
  if(rank != 0)
    return;

  // The initialization has all worked if we get to here on the rank=0
  // process. 
  EXPECT_TRUE(stan::math::mpi_cluster::is_listening());
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
  if(rank != 0)
    return;

  // perform simple check if a very basic mpi gather works over the
  // stan math mpi building blocks
  std::unique_lock<std::mutex> cluster_lock;
  EXPECT_NO_THROW((cluster_lock = stan::math::mpi_broadcast_command<
                       stan::math::mpi_distributed_apply<mpi_hello>>()));

  EXPECT_TRUE(cluster_lock.owns_lock());

  boost::mpi::communicator world;
  
  std::vector<int> world_ranks(world_size, -1);
  boost::mpi::gather(world, world.rank(), world_ranks, 0);

  for (int i = 0; i < world_size; ++i)
    EXPECT_EQ(world_ranks[i], i);
}

#endif
