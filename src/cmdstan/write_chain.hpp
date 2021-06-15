#ifndef CMDSTAN_WRITE_CHAIN_HPP
#define CMDSTAN_WRITE_CHAIN_HPP

#include <stan/callbacks/writer.hpp>
#include <stan/version.hpp>
#include <string>

namespace cmdstan {

inline void write_chain(stan::callbacks::writer& writer,
                        unsigned int chain_id) {
  writer("chain_id = " + std::to_string(chain_id));
}

}  // namespace cmdstan
#endif
