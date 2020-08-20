#ifndef CMDSTAN_CLI_HPP
#define CMDSTAN_CLI_HPP

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <CLI/CLI.hpp>
#include <stan/callbacks/writer.hpp>
#include <map>
#include <sstream>

namespace cmdstan {
  struct SharedOptions {
    int id = 0;
    std::string data_file = "";
    double init_radius = 2;
    std::string init_file = "";
    unsigned int seed
    = (boost::posix_time::microsec_clock::universal_time()
       - boost::posix_time::ptime(boost::posix_time::min_date_time))
      .total_milliseconds();
    std::string output_file = "output.csv";
    std::string diagnostic_file = "";
    int refresh = 100;
  };

  struct DiagnoseOptions {
    double epsilon = 1e-6;
    double threshold = 1e-6;
  };

  struct GenerateQuantitiesOptions {
    std::string fitted_params = "";
  };

  struct OptimizeOptions {
    enum class Algorithm : int { bfgs, lbfgs, newton };
    Algorithm algorithm{Algorithm::lbfgs};
    std::map<std::string, Algorithm> algorithm_map{{"bfgs", Algorithm::bfgs},
						   {"lbfgs", Algorithm::lbfgs},
						   {"newton", Algorithm::newton}};
    double init_alpha = 0.001;
    double tol_obj = 1e-12;
    double tol_rel_obj = 1e+4;
    double tol_grad = 1e-8;
    double tol_rel_grad = 1e+7;
    double tol_param = 1e-8;
    int history_size = 5;
    int iter = 2000;
    bool save_iterations = false;
  };

  struct SampleOptions {
    enum class Algorithm : int { nuts, hmc, fixed_param };
    std::map<std::string, Algorithm> algorithm_map{{"nuts", Algorithm::nuts},
						   {"hmc", Algorithm::hmc},
						   {"fixed_param", Algorithm::fixed_param}};
    Algorithm algorithm{Algorithm::nuts};

    int max_depth = 10;
    double int_time = 6.28318530717959;

    enum class Metric : int { unit, diag, dense };
    std::map<std::string, Metric> metric_map{{"unit", Metric::unit},
					     {"diag", Metric::diag},
					     {"dense", Metric::dense}};
    Metric metric{Metric::diag};
    std::string metric_file;

    double stepsize = 1;
    double stepsize_jitter = 0;

    int num_samples = 1000;
    int num_warmup = 1000;
    bool save_warmup = false;
    int thin = 1;
    bool adapt_off = false;

    double gamma = 0.05;
    double delta = 0.8;
    double kappa = 0.75;
    double t0 = 10;
    unsigned int init_buffer = 75;
    unsigned int term_buffer = 50;
    unsigned int window = 25;
  };

  struct VariationalOptions {
    enum class Algorithm : int { meanfield, fullrank };
    Algorithm algorithm{Algorithm::meanfield};
    std::map<std::string, Algorithm> algorithm_map{{"meanfield", Algorithm::meanfield},
						   {"fullrank", Algorithm::fullrank}};
    int iter = 10000;
    int grad_samples = 1;
    int elbo_samples = 100;
    double eta = 1.0;
    bool adapt_off = false;
    int adapt_iter = 50;
    double tol_rel_obj = 0.01;
    int eval_elbo = 100;
    int output_draws = 1000;
  };

  void setup_shared_options(CLI::App* app, SharedOptions& shared_options) {
    app
      ->add_option("--id", shared_options.id, "Unique process identifier")
      ->check(CLI::NonNegativeNumber);
    app
      ->add_option("-d,--data_file", shared_options.data_file, "Input data file")
      ->check(CLI::ExistingFile);

    auto init_range
      = app->add_option_group("Initialization radius",
			      "Initialization radius on the unconstrained scale");
    init_range
      ->add_option("--init_radius", shared_options.init_radius,
		   "initializes randomly between [-x, x]")
      ->check(CLI::NonNegativeNumber);
    init_range
      ->add_flag_function("--init_zero",
			  [&](int count) -> void{ shared_options.init_radius = 0; },
			  "initializes to 0");
    init_range->require_option(0, 1);

    app
      ->add_option("-i,--init_file", shared_options.init_file, "initialization file")
      ->check(CLI::ExistingFile);
    app
      ->add_option("-S,--seed", shared_options.seed, "Random number generator seed")
      ->check(CLI::PositiveNumber);
    app
      ->add_option("-o,--output_file", shared_options.output_file, "Output file");
    app
      ->add_option("--diagnostic_file", shared_options.diagnostic_file,
		   "Auxiliary output file for diagnostic information");
    app
      ->add_option("-r,--refresh", shared_options.refresh,
		   "Number of iterations between screen updates")
      ->check(CLI::NonNegativeNumber);
  }


  void setup_diagnose(CLI::App& app,
		      SharedOptions& shared_options,
		      DiagnoseOptions& diagnose_options) {
    CLI::App* diagnose
      = app.add_subcommand("diagnose",
			   "Check model gradient against finite differences");
    diagnose->alias("test");
    diagnose->alias("d");
    setup_shared_options(diagnose, shared_options);
    diagnose->add_option("-e,--epsilon", diagnose_options.epsilon,
			 "Finite difference step size")
      ->check(CLI::PositiveNumber);
    diagnose->add_option("-t,--threshold", diagnose_options.threshold,
			 "Error threshold")
      ->check(CLI::PositiveNumber);
  }

  void setup_generate_quantities(CLI::App& app,
				 SharedOptions& shared_options,
				 GenerateQuantitiesOptions& gq_options) {
    CLI::App* generate_quantities
      = app.add_subcommand("generate_quantities",
			   "Generate quantities of interest");
    generate_quantities->alias("gq");
    generate_quantities->alias("g");
    setup_shared_options(generate_quantities, shared_options);
    generate_quantities
      ->add_option("--fitted_params", gq_options.fitted_params,
		   "Input file of sample of fitted parameter values for model "
		   "conditioned on data")
      ->check(CLI::ExistingFile)
      ->required();
  }

  void setup_optimize(CLI::App& app,
		      SharedOptions& shared_options,
		      OptimizeOptions& optimize_options) {
    CLI::App* optimize = app.add_subcommand("optimize", "Point estimation");
    optimize->alias("optimise");
    optimize->alias("o");
    setup_shared_options(optimize, shared_options);
    auto algorithm
      = optimize->add_option_group("Algorithm", "Optimization algorithm");

    auto lbfgs = algorithm
      ->add_flag_function("--lbfgs",
			  [&](int count) -> void
			  { optimize_options.algorithm = OptimizeOptions::Algorithm::lbfgs; },
			  "Default: LBFGS with linesearch");
    auto bfgs = algorithm
      ->add_flag_function("--bfgs",
			  [&](int count) -> void
			  { optimize_options.algorithm = OptimizeOptions::Algorithm::bfgs; },
			  "BFGS with linesearch");
    auto newton = algorithm
      ->add_flag_function("--newton",
			  [&](int count) -> void
			  { optimize_options.algorithm = OptimizeOptions::Algorithm::newton; },
			  "Newton's method");
    algorithm->require_option(0, 1);

    optimize->add_option("-a,--init_alpha", optimize_options.init_alpha,
			 "Line search step size for first iteration")
      ->check(CLI::PositiveNumber)
      ->excludes(newton);
    optimize->add_option("--tol_obj", optimize_options.tol_obj,
			 "Convergence tolerance on absolute changes "
			 "in objective function value")
      ->check(CLI::PositiveNumber)
      ->excludes(newton);
    optimize->add_option("--tol_rel_obj", optimize_options.tol_rel_obj,
			 "Convergence tolerance on relative changes "
			 "in objective function value")
      ->check(CLI::PositiveNumber)
      ->excludes(newton);
    optimize->add_option("--tol_grad", optimize_options.tol_grad,
			 "Convergence tolerance on the norm of the gradient")
      ->check(CLI::PositiveNumber)
      ->excludes(newton);
    optimize->add_option("--tol_rel_grad", optimize_options.tol_rel_grad,
			 "Convergence tolerance on the relative norm of the gradient")
      ->check(CLI::PositiveNumber)
      ->excludes(newton);
    optimize->add_option("--tol_param", optimize_options.tol_param,
			 "Convergence tolerance on changes in parameter value")
      ->check(CLI::PositiveNumber)
      ->excludes(newton);

    optimize->add_option("--history_size", optimize_options.history_size,
			 "Amount of history to keep for L-BFGS")
      ->check(CLI::PositiveNumber)
      ->excludes(newton)
      ->excludes(bfgs);

    optimize->add_option("--iter", optimize_options.iter,
			 "Total number of iterations")
      ->check(CLI::PositiveNumber);
    optimize->add_flag("-s,--save_iterations", optimize_options.save_iterations,
		       "Stream optimization progress to output?");
  }

  void setup_sample(CLI::App& app,
		    SharedOptions& shared_options,
		    SampleOptions& sample_options) {
    CLI::App* sample
      = app.add_subcommand("sample",
			   "Bayesian inference with Markov Chain Monte Carlo");
    sample->alias("mcmc");
    sample->alias("s");
    setup_shared_options(sample, shared_options);

    auto algorithm
      = sample->add_option_group("Algorithm", "Algorithm for MCMC");

    auto nuts = algorithm
      ->add_flag_function("--nuts",
			  [&](int count) -> void
			  { sample_options.algorithm = SampleOptions::Algorithm::nuts; },
			  "Default: The No-U-Turn Sampler");
    auto hmc = algorithm
      ->add_flag_function("--hmc",
			  [&](int count) -> void
			  { sample_options.algorithm = SampleOptions::Algorithm::hmc; },
			  "Hamlitonian Monte Carlo (static)");
    sample->add_option("-T,--int_time", sample_options.int_time,
		       "Total integration time for Hamiltonian evolution")
      ->check(CLI::PositiveNumber)
      ->needs(hmc);
    sample->add_option("-m,--max_depth", sample_options.max_depth,
		       "Maximum tree depth")
      ->check(CLI::PositiveNumber)
      ->excludes(hmc);
    auto fixed_param = algorithm
      ->add_flag_function("--fixed_param",
			  [&](int count) -> void
			  { sample_options.algorithm = SampleOptions::Algorithm::fixed_param; },
			  "Fixed Parameter Sample");
    algorithm->require_option(0, 1);

    sample->add_option("--num_samples", sample_options.num_samples,
		       "Number of sampling iterations")
      ->check(CLI::NonNegativeNumber);
    sample->add_option("--num_warmup", sample_options.num_warmup,
		       "Number of warmup iterations")
      ->check(CLI::NonNegativeNumber)
      ->excludes(fixed_param);
    sample->add_flag("-s,--save_warmup", sample_options.save_warmup,
		     "Stream warmup samples to output?");
    sample->add_option("-t,--thin", sample_options.thin,
		       "Period between saved samples")
      ->check(CLI::NonNegativeNumber)
      ->excludes(fixed_param);

    auto adapt_off
      = sample->add_flag("--adapt_off", sample_options.adapt_off,
			 "Adaptation disengaged")
      ->excludes(fixed_param);
    sample->add_option("-g,--gamma", sample_options.gamma,
		       "Adaptation regularization scale")
      ->check(CLI::PositiveNumber)
      ->excludes(adapt_off)
      ->excludes(fixed_param);
    sample->add_option("-D,--delta", sample_options.delta,
		       "Adaptation target acceptance statistic")
      ->check(CLI::PositiveNumber & CLI::Range(0.0, 1.0))
      ->excludes(adapt_off)
      ->excludes(fixed_param);
    sample->add_option("-k,--kappa", sample_options.kappa,
		       "Adaptation relaxation exponent")
      ->check(CLI::PositiveNumber)
      ->excludes(adapt_off)
      ->excludes(fixed_param);
    sample->add_option("--t0", sample_options.t0, "Adaptation iteration offset")
      ->check(CLI::PositiveNumber)
      ->excludes(adapt_off)
      ->excludes(fixed_param);
    sample->add_option("--init_buffer", sample_options.init_buffer,
		       "Width of initial fast adaptation interval")
      ->excludes(adapt_off)
      ->excludes(fixed_param);
    sample->add_option("--term_buffer", sample_options.term_buffer,
		       "Width of final fast adaptation interval")
      ->excludes(adapt_off)
      ->excludes(fixed_param);
    sample->add_option("-w,--window", sample_options.window,
		       "Initial width of slow adaptation interval")
      ->excludes(adapt_off)
      ->excludes(fixed_param);

    sample->add_option("-M,--metric", sample_options.metric,
		       "Geometry of base manifold")
      ->transform(CLI::CheckedTransformer(sample_options.metric_map,
					  CLI::ignore_case))
      ->excludes(fixed_param);

    sample
      ->add_option("--metric_file", sample_options.metric_file,
		   "Input file with precomputed Euclidean metric")
      ->check(CLI::ExistingFile)
      ->excludes(fixed_param);

    sample
      ->add_option("--stepsize", sample_options.stepsize,
		   "Step size for discrete evolution")
      ->check(CLI::PositiveNumber)
      ->excludes(fixed_param);

    sample
      ->add_option("-j,--stepsize_jitter", sample_options.stepsize_jitter,
		   "Uniformly random jitter of the stepsize, in proportion")
      ->check(CLI::PositiveNumber & CLI::Range(0.0, 1.0))
      ->excludes(fixed_param);
  }

  void setup_variational(CLI::App& app,
			 SharedOptions& shared_options,
			 VariationalOptions& variational_options) {
    CLI::App* variational
      = app.add_subcommand("variational", "Variational inference");
    variational->alias("advi");
    variational->alias("v");
    setup_shared_options(variational, shared_options);
    auto algorithm
      = variational->add_option_group("Algorithm", "Variational inference algorithm");
    auto meanfield = algorithm
      ->add_flag_function("--meanfield",
			  [&](int count) -> void
			  { variational_options.algorithm = VariationalOptions::Algorithm::meanfield; },
			  "Default: Mean-field approximation");
    auto newton = algorithm
      ->add_flag_function("--fullrank",
			  [&](int count) -> void
			  { variational_options.algorithm = VariationalOptions::Algorithm::fullrank; },
			  "Full-rank covariance");
    algorithm->require_option(0, 1);

    variational->add_option("--iter", variational_options.iter,
			    "Maximum number of ADVI iterations")
      ->check(CLI::PositiveNumber);
    variational->add_option("--grad_samples", variational_options.grad_samples,
			    "Number of Monte Carlo draws for computing the gradient")
      ->check(CLI::PositiveNumber);
    variational->add_option("--elbo_samples", variational_options.elbo_samples,
			    "Number of Monte Carlo draws for estimate of ELBO")
      ->check(CLI::PositiveNumber);
    variational->add_option("--eta", variational_options.eta,
			    "Stepsize scaling parameter")
      ->check(CLI::PositiveNumber);
    auto adapt_off
      = variational->add_flag("--adapt_off", variational_options.adapt_off,
			      "Adaptation disengaged");
    variational->add_option("--adapt_iter", variational_options.adapt_iter,
			    "Number of iterations for eta adaptation")
      ->check(CLI::PositiveNumber)
      ->excludes(adapt_off);
    variational->add_option("--tol_rel_obj", variational_options.tol_rel_obj,
			    "Relative tolerance parameter for convergence")
      ->check(CLI::PositiveNumber);
    variational->add_option("--eval_elbo", variational_options.eval_elbo,
			    "Number of iterations between ELBO evaluations")
      ->check(CLI::PositiveNumber);
    variational->add_option("--output_draws", variational_options.output_draws,
			    "Number of approximate posterior output draws to save")
      ->check(CLI::PositiveNumber);
  }

  namespace internal {
    template <typename T>
    std::string option(const T& val, int count) {
      std::stringstream msg;
      msg << val << (count == 0 ? " (Default)" : "");
      return msg.str();
    }

    template <typename T>
    std::string enum_to_str(const T& val, std::map<std::string, T> map) {
      return std::find_if(map.begin(), map.end(),
			  [&](auto& it) { return it.second == val; })->first;
    }

    void print_old_command_header(CLI::App& app,
				  SharedOptions& shared_options,
				  stan::callbacks::writer& writer) {
      writer("id = " + internal::option(shared_options.id, app.count("--id")));
      writer("data");
      writer("  file = "
	     + option(shared_options.data_file, app.count("--data_file")));
      if (app.count("--init_radius") || app.count("--init_zero") || app.count("--init_file")) {
	if (app.count("--init_radius")) {
	  writer("init = " + option(shared_options.init_radius, app.count("--init_radius")));
	} else if (app.count("--init_zero")) {
	  writer("init = 0");
	} else {
	  writer("init = " + shared_options.init_file);
	}
      } else {
	writer("init = " + option(shared_options.init_radius, app.count("--init_radius")));
      }
      writer("random");
      writer("  seed = " + option(shared_options.seed, app.count("--seed")));
      writer("output");
      writer("  file = "
	     + option(shared_options.output_file, app.count("--output_file")));
      writer("  diagnostic_file = "
	     + option(shared_options.diagnostic_file, app.count("--diagnostic_file")));
      writer("  refresh = "
	     + option(shared_options.refresh, app.count("--refresh")));
    }
  }

  void print_old_command_header(CLI::App& app,
				SharedOptions& shared_options,
				SampleOptions& sample_options,
				stan::callbacks::writer& writer) {
    CLI::App* s = app.get_subcommands()[0];
    writer("method = sample (Default)");
    writer("  sample");
    writer("    num_samples = "
	   + internal::option(sample_options.num_samples, s->count("--num_samples")));
    writer("    num_warmup = "
	   + internal::option(sample_options.num_warmup, s->count("--num_warmup")));
    writer("    save_warmup = "
	   + internal::option(static_cast<int>(sample_options.save_warmup),
			      s->count("--save_warmup")));
    writer("    thin = "
	   + internal::option(sample_options.thin,
			      s->count("--thin")));
    writer("    adapt");
    writer("      engaged = "
	   + internal::option(static_cast<int>(!sample_options.adapt_off),
			      static_cast<int>(s->count("--adapt_off"))));
    writer("      gamma = "
	   + internal::option(sample_options.gamma, s->count("--gamma")));
    writer("      delta = "
	   + internal::option(sample_options.delta, s->count("--delta")));
    writer("      kappa = "
	   + internal::option(sample_options.kappa, s->count("--kappa")));
    writer("      t0 = "
	   + internal::option(sample_options.t0, s->count("--t0")));
    writer("      init_buffer = "
	   + internal::option(sample_options.init_buffer, s->count("--init_buffer")));
    writer("      term_buffer = "
	   + internal::option(sample_options.term_buffer, s->count("--term_buffer")));
    writer("      window = "
	   + internal::option(sample_options.window, s->count("--window")));
    if (sample_options.algorithm == SampleOptions::Algorithm::fixed_param) {
      writer("    algorithm = fixed_param");
    } else {
      writer("    algorithm = "
	     + internal::option("hmc", s->count("--hmc") + s->count("--nuts")));
      writer("      hmc");
      std::string algorithm = internal::enum_to_str(sample_options.algorithm, sample_options.algorithm_map);
      algorithm = algorithm == "hmc" ? "static" : "nuts";
      writer("        engine = "
	     + internal::option(algorithm, s->count("--nuts") + s->count("--hmc")));
      writer("          " + algorithm);
      if (sample_options.algorithm == SampleOptions::Algorithm::nuts) {
	writer("            max_depth = "
	       + internal::option(sample_options.max_depth, s->count("--max_depth")));
      } else if (sample_options.algorithm == SampleOptions::Algorithm::hmc) {
	writer("            int_time = "
	       + internal::option(sample_options.int_time, s->count("--int_time")));
      }
      std::string metric = internal::enum_to_str(sample_options.metric,
						 sample_options.metric_map);
      writer("        metric = "
	     + internal::option(metric + "_e", s->count("--metric")));
      writer("        metric_file = "
	     + internal::option(sample_options.metric_file, s->count("--metric_file")));
      writer("        stepsize = "
	     + internal::option(sample_options.stepsize, s->count("--stepsize")));
      writer("        stepsize_jitter = "
	     + internal::option(sample_options.stepsize_jitter, s->count("--stepsize_jitter")));
    }
    internal::print_old_command_header(*s, shared_options, writer);
  }

  void print_old_command_header(CLI::App& app,
				SharedOptions& shared_options,
				DiagnoseOptions& diagnose_options,
				stan::callbacks::writer& writer) {
    CLI::App* s = app.get_subcommands()[0];
    writer("method = diagnose");
    writer("  diagnose");
    writer("    test = gradient (Default)");
    writer("      gradient");
    writer("        epsilon = "
	   + internal::option(diagnose_options.epsilon, s->count("--epsilon")));
    writer("        error = "
	   + internal::option(diagnose_options.epsilon, s->count("--threshold")));
    internal::print_old_command_header(*s, shared_options, writer);
  }

  void print_old_command_header(CLI::App& app,
				SharedOptions& shared_options,
				GenerateQuantitiesOptions& gq_options,
				stan::callbacks::writer& writer) {
    CLI::App* s = app.get_subcommands()[0];
    writer("method = generate_quantities");
    writer("  generate_quantities");
    writer("    fitted_parameters = "
	   + internal::option(gq_options.fitted_params,
			      s->count("--fitted_params")));
    internal::print_old_command_header(*s, shared_options, writer);
  }

  void print_old_command_header(CLI::App& app,
				SharedOptions& shared_options,
				OptimizeOptions& optimize_options,
				stan::callbacks::writer& writer) {
    CLI::App* s = app.get_subcommands()[0];
    writer("method = optimize");
    writer("  optimize");
    std::string algorithm = internal::enum_to_str(optimize_options.algorithm,
						  optimize_options.algorithm_map);
    writer("    algorithm = "
	   + internal::option(algorithm, s->count("--lbfgs")
			      + s->count("--bfgs")
			      + s->count("--newton")));
    writer("      " + algorithm);
    if (optimize_options.algorithm == OptimizeOptions::Algorithm::lbfgs
	|| optimize_options.algorithm == OptimizeOptions::Algorithm::bfgs) {
      writer("        init_alpha = "
	     + internal::option(optimize_options.init_alpha, s->count("--init_alpha")));
      writer("        tol_obj = "
	     + internal::option(optimize_options.tol_obj, s->count("--tol_obj")));
      writer("        tol_rel_obj = "
	     + internal::option(optimize_options.tol_rel_obj, s->count("--tol_rel_obj")));
      writer("        tol_grad = "
	     + internal::option(optimize_options.tol_grad, s->count("--tol_grad")));
      writer("        tol_rel_grad = "
	     + internal::option(optimize_options.tol_rel_grad, s->count("--tol_rel_grad")));
      writer("        tol_param = "
	     + internal::option(optimize_options.tol_param, s->count("--tol_param")));
      if (optimize_options.algorithm == OptimizeOptions::Algorithm::lbfgs) {
	writer("        history_size = "
	       + internal::option(optimize_options.history_size, s->count("--history_size")));
      }
    }
    writer("    iter = "
	   + internal::option(optimize_options.iter, s->count("--iter")));
    writer("    save_iterations = "
	   + internal::option(static_cast<int>(optimize_options.save_iterations),
			      s->count("--save_iterations")));
    internal::print_old_command_header(*s, shared_options, writer);
  }

  void print_old_command_header(CLI::App& app,
				SharedOptions& shared_options,
				VariationalOptions& variational_options,
				stan::callbacks::writer& writer) {
    CLI::App* s = app.get_subcommands()[0];

    writer("method = variational");
    writer("  variational");
    std::string algorithm = internal::enum_to_str(variational_options.algorithm,
						  variational_options.algorithm_map);
    writer("    algorithm = "
	   + internal::option(algorithm, s->count("--meanfield") + s->count("--fullrank")));
    writer("      " + algorithm);
    writer("    iter = "
	   + internal::option(variational_options.iter, s->count("--iter")));
    writer("    grad_samples = "
	   + internal::option(variational_options.grad_samples, s->count("--grad_samples")));
    writer("    elbo_samples = "
	   + internal::option(variational_options.elbo_samples, s->count("--elbo_samples")));
    writer("    eta = "
	   + internal::option(variational_options.eta, s->count("--eta")));
    writer("    adapt");
    writer("      engaged = "
	   + internal::option(static_cast<int>(!variational_options.adapt_off),
			      s->count("--adapt_off")));
    writer("      iter = "
	   + internal::option(variational_options.adapt_iter, s->count("--adapt_iter")));
    writer("    tol_rel_obj = "
	   + internal::option(variational_options.tol_rel_obj, s->count("--tol_rel_obj")));
    writer("    eval_elbo = "
	   + internal::option(variational_options.eval_elbo, s->count("--eval_elbo")));
    writer("    output_samples = "
	   + internal::option(variational_options.output_draws, s->count("--output_draws")));
    internal::print_old_command_header(*s, shared_options, writer);
  }

}
#endif
