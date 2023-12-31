#include <cmdstan/command_helper.hpp>
#include <test/utility.hpp>
#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

using cmdstan::test::count_matches;
using cmdstan::get_path_separator;
using cmdstan::get_suffix;
using cmdstan::get_basename_suffix;

TEST(CommandHelper, filepath1) {
  std::string filepath = "foo" + std::string(1, get_path_separator())
                         + "bar" + std::string(1, get_path_separator())
                         + "foo.bar";
  EXPECT_EQ(get_suffix(filepath), ".bar");
}
