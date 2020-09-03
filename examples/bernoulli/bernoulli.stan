data {
  int N;
  int M;
}
parameters {
  matrix[N, M] A_param;
}
transformed parameters {
  vector[10] B_transform_param = A_param[1:10, 1];
}
model {
  for (i in 1:10) {
    B_transform_param[i] ~ normal(0, 1);
  }
}
