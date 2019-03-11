#ifndef CMDSTAN_WRITE_DRAW_HPP
#define CMDSTAN_WRITE_DRAW_HPP

#include <stan/callbacks/writer.hpp>
#include <vector>

namespace cmdstan {
  void write_draw(stan::callbacks::writer& writer,
                   const std::vector<double> draw) {
    writer(draw);
  }
}
#endif
