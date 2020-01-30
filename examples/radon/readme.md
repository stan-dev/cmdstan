- [Get the repo & depdencies](#org5557ab1)
  - [Edit make/local](#org32e15b8)
  - [Compile](#org5d3f69c)
  - [Run](#orgafcbab4)


<a id="org5557ab1"></a>

# Get the repo & depdencies

```bash
git clone --recurse-submodules --single-branch --branch mpi_warmup_framework git@github.com:stan-dev/cmdstan.git
```


<a id="org32e15b8"></a>

# Edit make/local

```bash
MPI_ADAPTED_WARMUP = 1
CXXFLAGS += -isystem /path/to/mpi/headers
TBB_CXX_TYPE=clang              # your cpp compiler
```


<a id="org5d3f69c"></a>

# Compile

```bash
make clean-all;
make -j4 examples/radon/radon
```


<a id="orgafcbab4"></a>

# Run

```bash
mpiexec -n 4 -l ./radon sample adapt num_cross_chains=4 cross_chain_window=100 data file=radon.data.R # MPICH
mpiexec -n 4 --tag-output ./radon sample adapt num_cross_chains=4 cross_chain_window=100 data file=radon.data.R # OpenMPI
```

runs 4 chains with cross-chain warmup, using 4 processes. Here

```bash
num_cross_chains=
```

specifies the number of parallel chains used in cross-chain warmup(default 4), and

```bash
cross_chain_window=
```

specifies that every `cross_chain_window` iterations(default 100) the convergence check is performed using data collected from the chains. See <https://discourse.mc-stan.org/t/new-adaptive-warmup-proposal-looking-for-feedback/12039> for details.
