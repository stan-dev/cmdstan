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
 * to the output.
 *
 * @param output stream to write output to
 * @param p reference to the map of profiles
 */
void write_profiling(std::ostream& output, stan::math::profile_map& p) {
  stan::math::profile_map::iterator it;

  output << "name,thread_id,total_time,forward_time,reverse_time,chain_"
            "stack,no_chain_stack,autodiff_calls,no_autodiff_calls"
         << std::endl;
  for (it = p.begin(); it != p.end(); it++) {
    output << it->first.first << "," << it->first.second << ","
           << (it->second.get_fwd_time() + it->second.get_rev_time()) << ","
           << it->second.get_fwd_time() << "," << it->second.get_rev_time()
           << "," << it->second.get_chain_stack_used() << ","
           << it->second.get_nochain_stack_used() << ","
           << it->second.get_num_rev_passes() << ","
           << it->second.get_num_no_AD_fwd_passes() << std::endl;
  }
}
}  // namespace cmdstan
#endif
