- [Get the repo & depdencies](#orgc68ef57)
  - [Edit make/local](#org510d4b1)
  - [Compile](#org03fc0e3)
  - [Run](#org54cb72d)


<a id="orgc68ef57"></a>

# Get the repo & depdencies

```bash
git clone --recurse-submodules --single-branch --branch mpi_warmup_framework git@github.com:stan-dev/cmdstan.git
```


<a id="org510d4b1"></a>

# Edit make/local

```bash
CXXFLAGS += -isystem /path/to/mpi/include -DSTAN_LANG_MPI -DMPI_ADAPTED_WARMUP
CC=mpicxx
CXX=mpicxx
TBB_CXX_TYPE=clang              # your cpp compiler
```


<a id="org03fc0e3"></a>

# Compile

```bash
make clean-all;
make -j4 examples/radon/radon
```


<a id="org54cb72d"></a>

# Run

```bash
mpiexec -n 4 -l ./radon sample data file=radon.data.R # MPICH
```

or

```bash
mpiexec -n 4 --tag-output ./radon sample data file=radon.data.R # OpenMPI
```

runs 4 chains with cross-chain warmup, using 4 processes.
