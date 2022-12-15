data {
  int<lower=0> N;
  array[N] int<lower=0, upper=1> y;
}
parameters {
  real<lower=0, upper=1> theta;
  real mu1;
  real mu2;
}
model {
  theta ~ beta(1, 1);
  y ~ bernoulli(theta);
  mu1 ~ normal(0, 10);
  mu2 ~ normal(0, 1);
}
generated quantities {
  real theta_copy = theta;
  array[N] int y_rep;
  for (n in 1 : N) {
    y_rep[n] = bernoulli_rng(theta);
  }
}

