#ifndef CMDSTAN_WRITE_PROFILING_HPP
#define CMDSTAN_WRITE_PROFILING_HPP

#include <stan/math/prim/fun/typedefs.hpp>
#include <string>

namespace cmdstan {

void write_profiling(stan::callbacks::writer &writer, stan::math::profilers& p) {
    writer("section,fun_eval_time,gradient_time,total_time");
    std::stringstream msg;
    for (auto const& x : p) {
        msg << x.first << "," << x.second.fwd_pass_time << "," << x.second.bckwd_pass_time << "," << (x.second.fwd_pass_time + x.second.bckwd_pass_time) << std::endl;
    }
    writer(msg.str());
}

}  // namespace cmdstan
#endif
