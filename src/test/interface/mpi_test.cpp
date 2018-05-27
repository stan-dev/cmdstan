#ifdef STAN_MPI

#include <stan/services/error_codes.hpp>
#include <cmdstan/command.hpp>
#include <gtest/gtest.h>
#include <string>
#include <test/utility.hpp>
#include <stdexcept>
#include <boost/math/policies/error_handling.hpp>
#include <stan/callbacks/stream_writer.hpp>
#include <src/test/test-models/logistic_map_rect.hpp>

using cmdstan::test::convert_model_path;

// google test for MPI requires to disable output for non-root
// (rank!=0) processes and adequate initialization of the MPI
// ressource
std::size_t rank;
std::size_t world_size;

// from:
// https://stackoverflow.com/questions/1706551/parse-string-into-argv-argc
// modified
void make_args(const char *args, int *argc, char ***aa) {
    char *buf = strdup(args);
    int c = 1;
    char *delim;
    char **argv = (char **)calloc(c, sizeof (char *));

    argv[0] = buf;

    while ((delim = strchr(argv[c - 1], ' '))) {
      argv = (char **)realloc(argv, (c + 1) * sizeof (char *));
      argv[c] = delim + 1;
      *delim = 0x00;
      c++;
    }

    *argc = c;
    *aa = argv;
}

class MPIEnvironment : public ::testing::Environment {
 public:
  virtual void SetUp() {
    std::vector<std::string> model_path;
    model_path.push_back("src");
    model_path.push_back("test");
    model_path.push_back("test-models");
    model_path.push_back("logistic_map_rect");
    std::vector<std::string> data_file_path;
    data_file_path.push_back("src");
    data_file_path.push_back("test");
    data_file_path.push_back("test-models");
    data_file_path.push_back("logistic_map_rect.dat.R");

    std::string command = convert_model_path(model_path) +
                          " method=sample num_samples=10 num_warmup=10" +
                          " data file=" + convert_model_path(data_file_path) +
                          " output file=test/output.csv";

    char **argv;
    int argc;

    make_args(command.c_str(), &argc, &argv);
    const char ** cargv = const_cast<const char **>(argv);
    
    cmdstan::command<stan_model>(argc, cargv);

    boost::mpi::communicator world;
    rank = world.rank();
    world_size = world.size();
    ::testing::TestEventListeners& listeners
        = ::testing::UnitTest::GetInstance()->listeners();
    if (rank != 0) {
      delete listeners.Release(listeners.default_result_printer());
    }
  }
  virtual void TearDown() {
  }

  virtual ~MPIEnvironment() {}
};

// register MPI global
::testing::Environment* const mpi_env
    = ::testing::AddGlobalTestEnvironment(new MPIEnvironment);


TEST(StanUiCommand, mpi_test) {
  if(rank != 0)
    return;

  // The initialization has all worked if we get to here on the rank=0
  // process. No more checks needed.
  EXPECT_TRUE(stan::math::mpi_cluster::is_listening());
}

#endif
