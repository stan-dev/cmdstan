transformed data {
  real udummy;
  {
    real v;
    array[10] real vv;
    vector[12] vvv;
    matrix[10, 10] vvvv;
    array[5] matrix[3, 4] vvvvv;
    print("transformed data: ", v, "  ", vv[1], "  ", vvv[2], "  ",
          vvvv[3, 4], "  ", vvvvv[1, 2, 3]);
  }
}
parameters {
  real y;
}
transformed parameters {
  real wdummy;
  {
    real w;
    array[10] real ww;
    vector[12] www;
    matrix[10, 10] wwww;
    array[5] matrix[3, 4] wwwww;
    print("transformed parameters: ", w, " ", ww[1], " ", www[2], " ",
          wwww[3, 4], " ", wwwww[1, 2, 3]);
  }
}
model {
  real z;
  array[10] real zz;
  vector[12] zzz;
  matrix[10, 10] zzzz;
  array[10] matrix[10, 10] zzzzz;
  print("model: ", z, " ", zz[1], " ", zzz[2], " ", zzzz[3, 4], " ",
        zzzzz[3, 4, 5]);
  y ~ normal(0, 1);
}

