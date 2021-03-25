data {
  int<lower=0> N;
  int<lower=0,upper=1> y[N];
}
parameters {
  real<lower=0,upper=1> theta;
}
model {
  profile("gahh") {
    theta ~ beta(1,1);  // uniform prior on interval 0,1
  }
  profile("YUaaa") {
    y ~ bernoulli(theta);    
  }
}
