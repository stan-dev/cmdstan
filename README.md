<a href="http://mc-stan.org">
<img src="https://raw.githubusercontent.com/stan-dev/logos/master/logo.png" width=200 alt="Stan Logo"/>
</a>

# CmdStan

<b>CmdStan</b> is the command line interface to Stan, a package providing

* full Bayesian inference using the No-U-Turn sampler (NUTS), a variant of Hamiltonian Monte Carlo (HMC),
* penalized maximum likelihood estimation (MLE) using optimization, either Newton or quasi-Newton algorithms BFGS and L-BFGS,
* approximate Bayesian inference using automatic differentiation variational inference (ADVI),
* a full first- and higher-order automatic differentiation library based on C++ template overloads, and
* a supporting fully-templated matrix, linear algebra, and probability special function library.

[![DOI](https://zenodo.org/badge/16967338.svg)](https://zenodo.org/badge/latestdoi/16967338)

### Home Page
Stan's home page, with links to everything you'll need to use Stan is:

[http://mc-stan.org/](http://mc-stan.org/)

### Interfaces
There are separate repositories here on GitHub for interfaces:
* [CmdStan](https://github.com/stan-dev/cmdstan) (command-line/shell interface)
* [CmdStanPy](https://github.com/stan-dev/cmdstanpy) (a lightweight interface to CmdStan for Python users)
* [CmdStanR](https://github.com/stan-dev/cmdstanr) (a lightweight interface to CmdStan for R users)
* [PyStan](https://github.com/stan-dev/pystan) (Python interface)
* [RStan](https://github.com/stan-dev/rstan) (R interface)

### Source Repository
CmdStan's source-code repository is hosted here on GitHub.

### Licensing
The Stan-to-C++ compiler written in OCaml, core Stan C++ code, and CmdStan are licensed under new BSD.

Note that the Stan math library depends on the Intel TBB library which is licensed under the Apache 2.0 license. This dependency implies an additional restriction as compared to the new BSD lincense alone. The Apache 2.0 license is incompatible with GPL-2 licensed code if distributed as a unitary binary. You may refer to the Apache 2.0 evaluation page on the [Stan Math wiki](https://github.com/stan-dev/math/wiki/Apache-2.0-License-Evaluation).

## Installation
1. Download the latest release tarball (use the "green" link) from: [CmdStan releases](https://github.com/stan-dev/cmdstan/releases)
2. Unpack the tarball.
3. From the folder, type `make` for a quick tutorial on how to build models.

## Installation using git
See [Getting Started with
CmdStan](https://github.com/stan-dev/cmdstan/wiki/Getting-Started-with-CmdStan) for instructions how to clone both CmdStan and Stan submodule.

## Troubleshooting

As of version 2.22, CmdStan has switched to the new Stan-to-C++ compiler, called [stanc3](https://github.com/stan-dev/stanc3).  This compiler is intended to be backwards compatible with the existing Stan language and should accept all models that compile under the release 2.21 compiler, (see this [list of bug fixes](https://github.com/stan-dev/stanc3/wiki/changes-from-stanc2)). 
The old C++ compiler is still available as program `bin/stanc2`.
This compiler is not longer being maintained, i.e., existing bugs will not be fixed
and new functions and features are only available in the stanc3 compiler.
Its intended use is as a diagnostic tool and backup for the new stanc3 compiler and

If you experience any problems or noticeable changes in compilation, execution, or outputs of model after switching to CmdStan 2.22, you can use the stanc2 compiler via the `make` option `STANC2`:
```
> make STANC2=TRUE my_program
```

If using the old compiler fixes the issues in your model, please report a bug on the [stanc3](https://github.com/stan-dev/stanc3) repository. Otherwise, report the issue to the [CmdStan](https://github.com/stan-dev/cmdstan) repository.

To permanently enable the stanc2 compiler, add the following to your `make/local` file:
```
STANC2=true
```
to the `make/local` file.

If using the old compiler fixes the issues in your model, please report a bug on the [stanc3](https://github.com/stan-dev/stanc3) repository. Otherwise, report the issue to the [CmdStan](https://github.com/stan-dev/cmdstan) repository.

Further information is available on the CmdStan wiki page https://github.com/stan-dev/cmdstan/wiki/Troubleshooting-the-stanc3-compiler
