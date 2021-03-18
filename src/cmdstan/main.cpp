#include <CLI/CLI.hpp>
#include <cmdstan/cli.hpp>
#include <cmdstan/command/diagnose.hpp>
#include <cmdstan/command/generate_quantities.hpp>
#include <cmdstan/command/optimize.hpp>
#include <cmdstan/command/sample.hpp>
#include <cmdstan/command/variational.hpp>
#include <cmdstan/return_codes.hpp>
#include <stan/math/prim/core/init_threadpool_tbb.hpp>
#include <stan/services/error_codes.hpp>

#ifdef STAN_MPI
#include <stan/math/prim/functor/mpi_cluster.hpp>
#include <stan/math/prim/functor/mpi_command.hpp>
#include <stan/math/prim/functor/mpi_distributed_apply.hpp>

stan::math::mpi_cluster &get_mpi_cluster() {
  static stan::math::mpi_cluster cluster;
  return cluster;
}
#endif

int main(int argc, const char *argv[]) {
  cmdstan::SharedOptions shared_options;
  cmdstan::SampleOptions sample_options;
  cmdstan::OptimizeOptions optimize_options;
  cmdstan::VariationalOptions variational_options;
  cmdstan::DiagnoseOptions diagnose_options;
  cmdstan::GenerateQuantitiesOptions generate_quantities_options;

  CLI::App app{"Stan program"};
  app.set_version_flag("-v,--version", std::string("CmdStan v") + CMDSTAN_VERSION + "; CLI11 prototype");

  app.require_subcommand(1);
  cmdstan::setup_sample(app, shared_options, sample_options);
  cmdstan::setup_optimize(app, shared_options, optimize_options);
  cmdstan::setup_variational(app, shared_options, variational_options);
  cmdstan::setup_diagnose(app, shared_options, diagnose_options);
  cmdstan::setup_generate_quantities(app, shared_options, generate_quantities_options);

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

#ifdef STAN_MPI
  stan::math::mpi_cluster &cluster = get_mpi_cluster();
  cluster.listen();
  if (cluster.rank_ != 0)
    return 0;
#endif
  stan::math::init_threadpool_tbb();

  std::string subcommand = app.get_subcommands()[0]->get_name();
  int return_code = 0;

#ifdef STAN_OPENCL
  if (app.get_subcommand()->count("--opencl_device")
      ^ app.get_subcommand()->count("--opencl_platform")) {
    std::cerr << "Please set both OpenCL device (--opencl_device) and platform (--opencl_platform) ids."
	      << std::endl;
    return stan::services::error_codes::USAGE;
  }
  if (app.get_subcommand()->count("--opencl_device") && app.get_subcommand()->count("--opencl_platform"))
    stan::math::opencl_context.select_device(shared_options.opencl_platform,
					     shared_options.opencl_device);
#endif

  try {
    int err_code;
    if (subcommand == "sample")
      err_code = cmdstan::sample(app, shared_options, sample_options);
    if (subcommand == "optimize")
      err_code = cmdstan::optimize(app, shared_options, optimize_options);
    if (subcommand == "variational")
      err_code = cmdstan::variational(app, shared_options,
				      variational_options);
    if (subcommand == "diagnose")
      err_code = cmdstan::diagnose(app, shared_options, diagnose_options);
    if (subcommand == "generate_quantities")
      err_code = cmdstan::generate_quantities(app, shared_options,
					      generate_quantities_options);
    export_profile_info(app, shared_options);
    if (err_code == 0)
      return cmdstan::return_codes::OK;
    else
      return cmdstan::return_codes::NOT_OK;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return cmdstan::return_codes::NOT_OK;
  }

#ifdef STAN_MPI
  cluster.stop_listen();
#endif
  return cmdstan::return_codes::OK;
}
