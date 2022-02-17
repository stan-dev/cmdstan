data { int N; }
parameters { vector[N] theta; }
model { target += sum(sin(theta)); }
