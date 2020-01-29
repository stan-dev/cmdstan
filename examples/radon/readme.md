- [Get the repo & depdencies](#org524895a)
  - [Edit make/local](#org07db622)
  - [Compile](#org7992c43)
  - [Run](#org3eb1b7d)


<a id="org524895a"></a>

# Get the repo & depdencies

```bash
git clone --recurse-submodules --single-branch --branch mpi_warmup_framework git@github.com:stan-dev/cmdstan.git
```


<a id="org07db622"></a>

# Edit make/local

```bash
MPI_ADAPTED_WARMUP = 1
CXXFLAGS += -isystem /path/to/mpi/include
TBB_CXX_TYPE=clang              # your cpp compiler
```


<a id="org7992c43"></a>

# Compile

```bash
make clean-all;
make -j4 examples/radon/radon
```


<a id="org3eb1b7d"></a>

# Run

```bash
mpiexec -n 4 -l ./radon sample num_cross_chains=4 data file=radon.data.R # MPICH
```

or

```bash
mpiexec -n 4 --tag-output ./radon sample num_cross_chains=4 data file=radon.data.R # OpenMPI
```

runs 4 chains with cross-chain warmup, using 4 processes.
