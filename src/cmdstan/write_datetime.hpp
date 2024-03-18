#ifndef CMDSTAN_WRITE_DATETIME_HPP
#define CMDSTAN_WRITE_DATETIME_HPP

#include <stan/callbacks/writer.hpp>
#include <stan/version.hpp>
#include <chrono>
#include <iomanip>
#include <string>

namespace cmdstan {

std::string current_datetime() {
  const std::time_t current_datetime
      = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::tm* curr_tm = std::gmtime(&current_datetime);
  std::stringstream current_datetime_msg;
  current_datetime_msg << std::setfill('0') << (1900 + curr_tm->tm_year) << "-"
                       << std::setw(2) << (curr_tm->tm_mon + 1) << "-"
                       << std::setw(2) << curr_tm->tm_mday << " "
                       << std::setw(2) << curr_tm->tm_hour << ":"
                       << std::setw(2) << curr_tm->tm_min << ":" << std::setw(2)
                       << curr_tm->tm_sec << " UTC";
  return current_datetime_msg.str();
}

void write_datetime(stan::callbacks::writer& writer) {
  std::stringstream current_datetime_msg;
  current_datetime_msg << "start_datetime = " << current_datetime();
  writer(current_datetime_msg.str());
}

}  // namespace cmdstan
#endif
