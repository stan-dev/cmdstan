data {
  int N;
  vector_cl[N] rating;
}
transformed data {
  vector_cl[N] a;
  int i = 2;
  real r = 2;

  a = beta(i, rating);
  a = fdim(i, rating);
  a = fmax(i, rating);
  a = fmin(i, rating);
  a = fmod(i, rating);
  a = hypot(i, rating);
  a = lbeta(i, rating);
  a = lchoose(i, rating);
  a = lmultiply(i, rating);
  a = log_diff_exp(i, rating);
  a = log_inv_logit_diff(i, rating);
  a = pow(i, rating);
  a = beta(rating, i);
  a = fdim(rating, i);
  a = fmax(rating, i);
  a = fmin(rating, i);
  a = fmod(rating, i);
  a = hypot(rating, i);
  a = lbeta(rating, i);
  a = lchoose(rating, i);
  a = lmultiply(rating, i);
  a = log_diff_exp(rating, i);
  a = log_inv_logit_diff(rating, i);
  a = pow(rating, i);
  a = tail(rating, i);

  a = add(r, rating);
  a = beta(r, rating);
  a = elt_divide(r, rating);
  a = fdim(r, rating);
  a = fmax(r, rating);
  a = fmin(r, rating);
  a = fmod(r, rating);
  a = hypot(r, rating);
  a = lbeta(r, rating);
  a = lchoose(r, rating);
  a = lmultiply(r, rating);
  a = log_diff_exp(r, rating);
  a = log_inv_logit_diff(r, rating);
  a = multiply(r, rating);
  a = pow(r, rating);
  a = subtract(r, rating);

  a = add(rating, r);
  a = beta(rating, r);
  a = elt_divide(rating, r);
  a = fdim(rating, r);
  a = fmax(rating, r);
  a = fmin(rating, r);
  a = fmod(rating, r);
  a = hypot(rating, r);
  a = lbeta(rating, r);
  a = lchoose(rating, r);
  a = lmultiply(rating, r);
  a = log_diff_exp(rating, r);
  a = log_inv_logit_diff(rating, r);
  a = multiply(rating, r);
  a = pow(rating, r);
  a = subtract(rating, r);

  a = segment(rating, i, i);
  a = sub_col(rating, i, i, i);
}
parameters {
  real y;
}
model {
   y ~ std_normal();
}
