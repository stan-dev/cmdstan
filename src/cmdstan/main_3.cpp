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
  double sd = 0.5;

  for (int n = 0; n < N; ++n) {
    std::cout << "iteration " << n << std::endl;

    Eigen::Matrix<stan::math::var, -1, 1> theta(1);
    theta(0) = stan::math::normal_rng(0.0, sd, rng);
    
    stan::math::var lp = model.log_prob(theta, &std::cout);
    
    std::cout << "theta =         " << theta << std::endl;
    std::cout << "lp =            " << lp << std::endl;
    
    std::vector<double> gradients;
    std::vector<stan::math::var> vars;
    vars.push_back(theta(0));
    lp.grad(vars, gradients);
    std::cout << "d_lp / d_theta = " << gradients[0] << std::endl;
    
    stan::math::recover_memory();
  }
  

  return 0;
}
