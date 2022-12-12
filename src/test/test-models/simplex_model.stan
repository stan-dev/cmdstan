transformed data {
  int K = 3;
  vector[K] alpha = [1.0, 1.0, 1.0]';
}
parameters {
  array[K] simplex[K] theta; // transit probs
}
model {
  for (k in 1 : K) {
    theta[k] ~ dirichlet(alpha);
  }
}
