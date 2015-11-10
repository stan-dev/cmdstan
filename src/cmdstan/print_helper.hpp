#ifndef CMDSTAN_PRINT_HELPER_HPP
#define CMDSTAN_PRINT_HELPER_HPP

#include <iostream>

void print_deprecated() {
  std::cout << std::endl << std::endl;
  std::cout << "*** print is deprecated and will be removed in v3.0;"
            << std::endl
            << "*** use stansummary instead"
            << std::endl
            << std::endl;
}

void print_usage() {
  std::cout << "USAGE:  print <filename 1> [<filename 2> ... <filename N>]"
            << std::endl
            << std::endl;

  std::cout << "OPTIONS:" << std::endl << std::endl;
  std::cout << "  --autocorr=<chain_index>\tAppend the autocorrelations "
            << "for the given chain"
            << std::endl
            << std::endl;
  std::cout << "  --sig_figs=<int>\tSet significant figures of output "
            << "(Defaults to 2)"
            << std::endl
            << std::endl;
}

#endif
