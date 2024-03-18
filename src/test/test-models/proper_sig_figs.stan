parameters {
  real mu;
}
model {
  mu ~ normal(0, 1);
}
generated quantities {
  real p = 0.123456789123456789;
}

