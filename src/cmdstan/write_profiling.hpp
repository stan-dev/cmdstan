#ifndef CMDSTAN_WRITE_PROFILING_HPP
#define CMDSTAN_WRITE_PROFILING_HPP

#include <stan/math/rev/core/profiling.hpp>
#include <stan/callbacks/writer.hpp>
#include <stan/version.hpp>
#include <string>
#include <vector>

namespace cmdstan {

/**
 * Writes the data from the map of profiles in a CSV format
 * to the supplied writer.
 * 
 * @param writer object of a Stan writer class to write to.
 * @param p reference to the map of profiles
 */
void write_profiling(stan::callbacks::writer& writer,
                     stan::math::profile_map& p) {
  stan::math::profile_map::iterator it;
  std::stringstream profile_csv_stream;
  profile_csv_stream << "name,thread_id,time,forward_time,reverse_time,chain_"
                        "stack,no_chain_stack,autodiff_calls,no_autodiff_calls"
                     << std::endl;
  for (it = p.begin(); it != p.end(); it++) {
    profile_csv_stream
        << it->first.first
        << ","
        << it->first.second
        << ","
        << (it->second.get_fwd_time() + it->second.get_rev_time())
        << ","
        << it->second.get_fwd_time()
        << ","
        << it->second.get_rev_time()
        << ","
        << it->second.get_chain_stack_used()
        << ","
        << it->second.get_nochain_stack_used()
        << ","
        << it->second.get_num_rev_passes()
        << ","
        << it->second.get_num_no_AD_fwd_passes() << std::endl;
  }
  writer(profile_csv_stream.str());
}
}  // namespace cmdstan
#endif
