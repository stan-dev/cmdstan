#include <stan/io/dump.hpp>
#include <stan/model/model_base.hpp>
#include <iostream>
#include <fstream>

stan::model::model_base& new_model(stan::io::var_context& data_context,
                                   unsigned int seed,
                                   std::ostream* msg_stream);

int main(int argc, const char* argv[]) {
  std::ifstream data_file("examples/bernoulli/bernoulli.data.R");

  stan::io::dump context(data_file);
  
  stan::model::model_base& model = new_model(context, 1, NULL);

  std::cout << "model name: " << model.model_name() << std::endl;

  Eigen::VectorXd theta(1);
  theta << 0;

  std::cout << "eval:       " << model.log_prob(theta, &std::cout) << std::endl;
  // R
  // > dbinom(0, 1, 0.5, log=TRUE)
  // [1] -0.6931472

  return 0;
}
