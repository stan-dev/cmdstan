#include <stan/io/dump.hpp>
#include <stan/model/model_base.hpp>
#include <iostream>
#include <fstream>
#include <random>

stan::model::model_base& new_model(stan::io::var_context& data_context,
                                   unsigned int seed,
                                   std::ostream* msg_stream);

int main(int argc, const char* argv[]) {
  std::ifstream data_file("examples/bernoulli/bernoulli.data.R");

  stan::io::dump context(data_file);
  
  stan::model::model_base& model = new_model(context, 1, NULL);

  std::cout << "model name: " << model.model_name() << std::endl;


  std::mt19937 rng;
  std::uniform_real_distribution<double> unif(0, 1);

  int N = 1000;
  double sd = 0.2;
  
  Eigen::VectorXd theta(N);
  Eigen::VectorXd lp(N);
  theta(0) = stan::math::normal_rng(0.0, sd, rng);
  Eigen::VectorXd theta_prime(1);
  theta_prime(0) = theta(0);
  lp(0) = model.log_prob(theta_prime, &std::cout);
  
  for (int n = 1; n < N; ++n) {
    std::cout << "iteration " << n << std::endl;
    theta_prime(0) = stan::math::normal_rng(theta(n - 1), sd, rng);
    double lp_prime = model.log_prob(theta_prime, &std::cout);

    if ((lp_prime > lp[n - 1])
	|| (unif(rng) <= std::exp(lp_prime - lp[n - 1]))) {
      theta(n) = theta_prime(0);
      lp(n) = lp_prime;
    } else {
      theta(n) = theta(n - 1);
      lp(n) = lp(n - 1);
    }
  }

  std::cout << "theta = " << theta << std::endl;
  std::cout << "lp =    " << lp << std::endl;

  return 0;
}
