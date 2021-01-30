data {
  int <lower=0> J; // number of schools
  real y[J]; // estimated treatment
  real<lower=0> sigma[J]; // std of estimated effect
}
parameters {
  vector[J] theta_trans; // transformation of theta
  real mu; // hyper-parameter of mean
  real<lower=0> tau; // hyper-parameter of sd
}
transformed parameters{
  vector[J] theta;
  // original theta
  theta=theta_trans*tau+mu;
}
model {
  theta_trans ~ normal (0,1);
  y ~ normal(theta , sigma);
  mu ~ normal(0, 5); // a non-informative prior
  tau ~ cauchy(0, 5);
}