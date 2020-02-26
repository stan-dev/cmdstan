data {
  int <lower=0> N;
  int <lower=0> D;
  matrix [N, D] X;
  vector [N] y;
}

parameters {
  vector [D] beta;
  real <lower=0> sigma;
}

model {
  // prior
  target += normal_lpdf(beta | 0, 10);
  target += normal_lpdf(sigma | 0, 10);
  // likelihood
  target += normal_lpdf(y | X * beta, sigma);
}
