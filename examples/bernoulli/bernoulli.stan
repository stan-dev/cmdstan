data {
  int<lower=0> N;
  array[N] int<lower=0,upper=1> y; // or int<lower=0,upper=1> y[N];
}
parameters {
  real<lower=0,upper=1> theta;
  real<lower=0,upper=1> theta2;
}
model {
  theta ~ beta(1,1);  // uniform prior on interval 0,1
  theta2 ~ beta(1,1);  // uniform prior on interval 0,1
  y ~ bernoulli(theta);
}
