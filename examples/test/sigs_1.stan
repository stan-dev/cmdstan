data {
  int N;
  vector_cl[N] rating;
}
transformed data {
  vector_cl[N] a;
  real b;
  int c;

  // (vector) => vector
  // a = Phi(rating);
  // a = Phi_approx(rating);
  // a = acos(rating);
  // a = acosh(rating);
  // a = asin(rating);
  // a = asinh(rating);
  // a = atan(rating);
  // a = atanh(rating);
  // a = cbrt(rating);
  // a = ceil(rating);
  // a = cos(rating);
  // a = cosh(rating);
  // a = cumulative_sum(rating);
  // a = digamma(rating);
  // a = erf(rating);
  // a = erfc(rating);
  // a = exp(rating);
  // a = exp2(rating);
  // a = expm1(rating);
  // a = fabs(rating);
  // a = floor(rating);
  // a = inv(rating);
  // a = inv_Phi(rating);
  // a = inv_cloglog(rating);
  // a = inv_logit(rating);
  // a = inv_sqrt(rating);
  // a = inv_square(rating);
  // a = lgamma(rating);
  // a = log(rating);
  // a = log10(rating);
  // a = log1m(rating);
  // a = log1m_exp(rating);
  // a = log1m_inv_logit(rating);
  // a = log1p(rating);
  // a = log1p_exp(rating);
  // a = log2(rating);
  // a = log_inv_logit(rating);
  // a = log_softmax(rating);
  // a = logit(rating);
  // a = minus(rating);
  // a = plus(rating);
  // a = reverse(rating);
  // a = round(rating);
  // a = rows_dot_self(rating);
  // a = sin(rating);
  // a = sinh(rating);
  // a = softmax(rating);
  // a = sort_asc(rating);
  // a = sort_desc(rating);
  // a = sqrt(rating);
  // a = square(rating);
  // a = tan(rating);
  // a = tanh(rating);
  // a = tgamma(rating);
  // a = trigamma(rating);
  // a = trunc(rating);

  // // (vector) => real
  // b = dot_self(rating);
  // b = log_sum_exp(rating);
  // b = mean(rating);
  // b = prod(rating);
  // b = sd(rating);
  // b = sum(rating);
  // b = variance(rating);

  // // (vector) => int
  // c = cols(rating);
  // c = num_elements(rating);
  // c = rows(rating);
  // c = size(rating);

}
parameters {
  real y;
}
transformed parameters {
  
}
model {
   vector_cl[N] ap_cl;

   ap_cl = Phi(rating);  
   y ~ std_normal();
}
