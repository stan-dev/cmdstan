data {
  int N;
  int M;
  int nz_vals;
  int nz_rows[nz_vals];
  int nz_cols[nz_vals];
  sparse_matrix[nz_rows, nz_cols, N, M] x;
}


parameters {
  sparse_matrix[nz_rows, nz_cols, N, M] VV;
}

transformed parameters {
  sparse_matrix[N, M] ZZ = VV * nz_vals;
}
