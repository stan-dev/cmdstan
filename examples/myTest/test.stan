data {
   int<lower=0> N;
   array[N] int<lower=0,upper=1> y;
}
parameters {
   real<lower=0,upper=1> theta;
}
model {
   theta ~ beta(1,1);  // uniform prior on interval 0,1
   y ~ bernoulli(theta);
}
generated quantities {
   complex z = 5;
   real r = get_real(z);
   vector[4] u;
}