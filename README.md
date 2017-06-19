<a href="http://mc-stan.org">
<img src="https://raw.githubusercontent.com/stan-dev/logos/master/logo.png" width=200 alt="Stan Logo"/>
</a>

# CmdStan

<b>CmdStan</b> is the command line interface to Stan, a C++ package providing

* full Bayesian inference using the No-U-Turn sampler (NUTS), a variant of Hamiltonian Monte Carlo (HMC),
* approximate Bayesian inference using automatic differentiation variational inference (ADVI),
* penalized maximum likelihood estimation (MLE) using L-BFGS optimization,
* a full first- and higher-order automatic differentiation library based on C++ template overloads, and
* a supporting fully-templated matrix, linear algebra, and probability special function library.

[![DOI](https://zenodo.org/badge/16967338.svg)](https://zenodo.org/badge/latestdoi/16967338)

### Home Page
Stan's home page, with links to everything you'll need to use Stan is:

[http://mc-stan.org/](http://mc-stan.org/)

### Interfaces
There are separate repositories here on GitHub for interfaces:
* RStan (R interface)
* PyStan (Python interface)
* CmdStan (command-line/shell interface)

### Source Repository
CmdStan's source-code repository is hosted here on GitHub.

### Licensing
The core Stan C++ code and CmdStan are licensed under new BSD.


## Installation
1. Download the latest release tarball (use the "green" link) from: [CmdStan releases](https://github.com/stan-dev/cmdstan/releases)
2. Unpack the tarball.
3. From the folder, type `make` for a quick tutorial on how to build models.

## Installation using git
See [Getting Started with
CmdStan](https://github.com/stan-dev/cmdstan/wiki/Getting-Started-with-CmdStan) for instructions how to clone both CmdStan and Stan submodule.
