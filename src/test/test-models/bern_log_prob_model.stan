data {
  int<lower=0> N;
  array[N] int<lower=0,upper=1> y;
}
parameters {
  real<lower=0,upper=1> theta;
  vector[2] mu_vector;
  row_vector[2] mu_row_vector;
  array[2] real mu_array_1d;
  matrix[2, 2] mu_matrix;
  array[2] matrix[2, 2] mu_matrix_array;
}
model {
  theta ~ beta(1,1);
  y ~ bernoulli(theta);
  mu_vector ~ std_normal();
  mu_row_vector ~ std_normal();
  to_vector(mu_array_1d) ~ std_normal();
  to_vector(mu_matrix) ~ std_normal();
  to_vector(mu_matrix_array[1]) ~ std_normal();
  to_vector(mu_matrix_array[2]) ~ std_normal();
}
generated quantities {
  real theta_copy = theta;
  array[N] int y_rep;
  for (n in 1:N)
    y_rep[n] = bernoulli_rng(theta);
}
