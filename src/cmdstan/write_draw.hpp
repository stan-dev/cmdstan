#ifndef CMDSTAN_WRITE_DRAW_HPP
#define CMDSTAN_WRITE_DRAW_HPP

#include <stan/callbacks/writer.hpp>
#include <vector>

namespace cmdstan {
  void write_draw(stan::callbacks::writer& writer,
                   const std::vector<double> draw) {
    writer(draw);
  }


  // write sample - add last column
  // row from sample is EigenVector
  // then append chain id
  void write_draw_chain_id(stan::callbacks::writer& writer,
                   const std::vector<double> draw) {
    // don't need to worry about final comma - simpler
    //        typename std::vector<T>::const_iterator last = v.end();
    //        --last;
    //        for (typename std::vector<T>::const_iterator it = v.begin();
    //             it != last; ++it)
    //          output_ << *it << ",";
    //        output_ << v.back() << std::endl;

 std::vector<float> v3(&v2[0], v2.data()+v2.cols()*v2.rows());


    //    writer(draw);
  }


}
#endif
