data {
  int<lower=0> N; 
  int<lower=0> J; 
  int<lower=1,upper=J> county[N];
  vector[N] y;
}

parameters {
  vector[J] a;
  real mu_a;
  real<lower=0,upper=100> sigma_a;
  real<lower=0,upper=100> sigma_y;
} 

model {
  vector[N] y_hat;
  for (i in 1:N)
    y_hat[i] = a[county[i]];

  mu_a ~ normal(0, 1);
  a ~ normal (mu_a, sigma_a);
  y ~ normal(y_hat, sigma_y);
}
