data {
  int N;
  vector_cl[N] rating;
}
transformed data {
  vector_cl[N] a;
  vector_cl[N] b;
  vector_cl[N] c;
  real d;
  c = add(a, b);
  c = append_row(a, b);
  c = beta(a, b);
  c = elt_divide(a, b);
  c = elt_multiply(a, b);
  c = fdim(a, b);
  c = fmax(a, b);
  c = fmin(a, b);
  c = fmod(a, b);
  c = hypot(a, b);
  c = lbeta(a, b);
  c = lchoose(a, b);
  c = lmultiply(a, b);
  c = log_diff_exp(a, b);
  c = log_inv_logit_diff(a, b);
  c = pow(a, b);
  c = rows_dot_product(a, b);
  c = subtract(a, b);

  d = distance(a, b);
  d = dot_product(a, b);
  d = log_mix(a, b);
  d = squared_distance(a, b);
}
parameters {
  real y;
}
model {
   y ~ std_normal();
}

