data {
  int N;
  int M;
  int nz_vals;
  int nz_rows[nz_vals];
  int nz_cols[nz_vals];
  sparse_matrix[nz_rows, nz_cols, N, M] x;
  // Should not be allowed!
  sparse_matrix[N, M] bad_x;
}

transformed data {
  sparse_matrix[N, M] xx = x * transpose(x);
  // Should be allowed
  //real data_cool = xx[1, 1];
  // Should not be allowed
  //xx[1, 1] = 5.0;
  xx[1:10, 1:10] = x[1:10, 1:10];

}


parameters {
  sparse_matrix[nz_rows, nz_cols, N, M] VV;
}

transformed parameters {
  sparse_matrix[N, M] ZZ = VV * nz_vals;
  // Cool and fine
  //real param_cool = ZZ[1, 1];
  // Neither are cool or fine
  //ZZ[1, 1] = 10.0;
  ZZ[1:10, 1:10] = VV[1:10, 1:10];
}
