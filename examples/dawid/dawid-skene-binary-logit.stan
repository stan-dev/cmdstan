// The ordering of logit_alpha constrains annotators to be more likely to
// produce a positive label from a positive instance than from a negative
// instance.   This is equivalent to requiring AUC > 0.5 for annotators.

data {
  int<lower = 0> N;                 // # observations
  int<lower = 0> I;                 // # items
  int<lower = 0> J;                 // # annotators
  int<lower = 1, upper = I> ii[N];  // item
  int<lower = 1, upper = J> jj[N];  // annotator
  int<lower = 0, upper = 1> y[N];   // label (0:neg, 1:pos)
}
parameters {
  real logit_pi;                    // log odds of prevalence of positive response
  ordered[2] logit_alpha[J];        // log odds of coder responses (1:neg, 2:pos)
}
model {
  real log_pi = log_inv_logit(logit_pi);
  real log1m_pi = log_inv_logit(-logit_pi);
  real zero = 0;
  matrix[I, 2] lp = rep_matrix(zero, I, 2);
  for (n in 1:N) {
    lp[ii[n], 1] += bernoulli_logit_lpmf(y[n] | logit_alpha[jj[n], 2]);
    lp[ii[n], 2] += bernoulli_logit_lpmf(y[n] | logit_alpha[jj[n], 1]);
  }
  for (i in 1:I)
    target += log_mix(inv_logit(logit_pi), lp[i, 1], lp[i, 2]);

  logit_pi ~ normal(0, 1.8);          // roughly uniform on pi (scale matches std logistic)
  logit_alpha[ , 1] ~ normal(-1, 2);  // centered at 73% specificity
  logit_alpha[ , 2] ~ normal(1, 2);   // centered at 73% sensitivity
}
