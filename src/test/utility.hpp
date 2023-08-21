#ifndef TEST__MODELS__UTILITY_HPP
#define TEST__MODELS__UTILITY_HPP

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <sys/stat.h>

namespace cmdstan {
namespace test {

// only counts non-overlapping matches;  after match, advances to
// end of match;
// empty target returns -1
int count_matches(const std::string &target, const std::string &s) {
  if (target.size() == 0)
    return -1;  // error
  int count = 0;
  for (size_t pos = 0; (pos = s.find(target, pos)) != std::string::npos;
       pos += target.size())
    ++count;
  return count;
}

/**
 * Gets the path separator for the OS.
 *
 * @return '\' for Windows, '/' otherwise.
 */
char get_path_separator() {
#if defined(WIN32) || defined(_WIN32) \
    || defined(__WIN32) && !defined(__CYGWIN__)
  static char path_separator = '\\';
#else
  static char path_separator = '/';
#endif
  return path_separator;
}

/**
 * Multiple command separator
 *
 * @return '&' for Windows, ';' otherwise.
 */
char multiple_command_separator() {
  if (get_path_separator() == '/')
    return ';';
  else
    return '&';
}

/**
 * Returns the path as a string with the appropriate
 * path separator.
 *
 * @param model_path vector of strings representing path to the model
 *
 * @return the string representation of the path with the appropriate
 *    path separator.
 */
std::string convert_model_path(const std::vector<std::string> &model_path) {
  std::string path;
  if (model_path.size() > 0) {
    path.append(model_path[0]);
    for (size_t i = 1; i < model_path.size(); i++) {
      path.append(1, get_path_separator());
      path.append(model_path[i]);
    }
  }
  return path;
}

struct run_command_output {
  std::string command;
  std::string output;
  long time;
  int err_code;
  bool hasError;
  std::string header;
  std::string body;

  run_command_output(const std::string command, const std::string output,
                     const long time, const int err_code)
      : command(command),
        output(output),
        time(time),
        err_code(err_code),
        hasError(err_code != 0),
        header(),
        body() {
    size_t end_of_header = output.find("\n\n");
    if (end_of_header == std::string::npos)
      end_of_header = 0;
    else
      end_of_header += 2;
    header = output.substr(0, end_of_header);
    body = output.substr(end_of_header);
  }

  run_command_output()
      : command(),
        output(),
        time(0),
        err_code(0),
        hasError(false),
        header(),
        body() {}
};

std::ostream &operator<<(std::ostream &os, const run_command_output &out) {
  os << "run_command output:"
     << "\n"
     << "- command:   " << out.command << "\n"
     << "- output:    " << out.output << "\n"
     << "- time (ms): " << out.time << "\n"
     << "- err_code:  " << out.err_code << "\n"
     << "- hasError:  " << (out.hasError ? "true" : "false") << "\n"
     << "- header:    " << out.header << "\n"
     << "- body:      " << out.body << std::endl;
  return os;
}

/**
 * Runs the command provided and returns the system output
 * as a string.
 *
 * @param command A command that can be run from the shell
 * @return the system output of the command
 */
run_command_output run_command(std::string command) {
  using boost::posix_time::microsec_clock;
  using boost::posix_time::ptime;
  FILE *in;
  std::string command_plus = command + " 2>&1";  // put stderr to stdout
  in = popen(command_plus.c_str(), "r");

  if (!in) {
    std::string err_msg;
    err_msg = "Fatal error with popen; could not execute: \"";
    err_msg += command;
    err_msg += "\"";
    throw std::runtime_error(err_msg.c_str());
  }

  std::string output;
  char buf[1024];
  size_t count;
  ptime time_start(microsec_clock::universal_time());  // start timer
  while ((count = fread(&buf, 1, 1024, in)) > 0)
    output += std::string(&buf[0], &buf[count]);
  ptime time_end(microsec_clock::universal_time());  // end timer

  // bits 15-8 is err code, bit 7 if core dump, bits 6-0 is signal number
  int err_code = pclose(in);
  if (err_code != 0 && (err_code >> 8) > 0)
    err_code >>= 8;
  if (err_code > 127) {  // covers stan_services and CLI11 - not segfault
    std::string err_msg;
    err_msg = "Command threw unexpected error: \"";
    err_msg += command;
    err_msg += "\"";
    throw std::runtime_error(err_msg.c_str());
  }
  return run_command_output(
      command, output, (time_end - time_start).total_milliseconds(), err_code);
}

/**
 * Returns the help options from the string provided.
 * Help options start with "--".
 *
 * @param help_output output from "model/command --help"
 * @return a vector of strings of the help options
 */
std::vector<std::string> parse_help_options(const std::string &help_output) {
  std::vector<std::string> help_options;

  size_t option_start = help_output.find("--");
  while (option_start != std::string::npos) {
    // find the option name (skip two characters for "--")
    option_start += 2;
    size_t option_end = help_output.find_first_of("= ", option_start);
    help_options.push_back(
        help_output.substr(option_start, option_end - option_start));
    option_start = help_output.find("--", option_start + 1);
  }

  return help_options;
}

/**
 * Parses output from a Stan model run from the command line.
 * Returns option, value pairs.
 *
 * @param command_output The output from a Stan model run from the command line.
 *
 * @return Option, value pairs as indicated by the Stan model.
 */
std::vector<std::pair<std::string, std::string>> parse_command_output(
    const std::string &command_output) {
  using std::pair;
  using std::string;
  using std::vector;
  vector<pair<string, string>> output;

  string option, value;
  size_t start = 0, end = command_output.find("\n", start);

  start = end + 1;
  end = command_output.find("\n", start);
  size_t equal_pos = command_output.find("=", start);

  while (equal_pos != string::npos) {
    using boost::trim;
    option = command_output.substr(start, equal_pos - start);
    value = command_output.substr(equal_pos + 1, end - equal_pos - 1);
    trim(option);
    trim(value);
    output.push_back(pair<string, string>(option, value));
    start = end + 1;
    end = command_output.find("\n", start);
    equal_pos = command_output.find("=", start);
  }
  return output;
}

/**
 * Read Stan CSV file into vector of doubles.
 * Expects comment line prefix '#' and header row.
 *
 * @param path Path to file.
 * @param config Vector for initial comment lines (CmdStan config)
 * @param header Vector for column header
 * @param cells Vector of output data, serialized by row.
 */
void parse_sample(const std::string &path, std::vector<std::string> &config,
                  std::vector<std::string> &header,
                  std::vector<double> &cells) {
  std::ifstream in;
  in.open(path);
  std::string line;
  while (in.peek() == '#') {
    std::getline(in, line);
    config.push_back(line);
  }
  std::getline(in, line);
  header.push_back(line);
  while (std::getline(in, line)) {
    if (line[0] == '#')
      continue;
    std::stringstream linestream(line);
    std::string cell;
    while (std::getline(linestream, cell, ',')) {
      cells.push_back(std::stold(cell));
    }
  }
  in.close();
}

/**
 * Given vector of strings, return index of first element
 * which contains a specified substring.
 *
 * @param lines Vector of strings to check
 * @param substring String to match on
 *
 * @return index of first line, or -1 if not found.
 */
int idx_first_match(const std::vector<std::string> &lines,
                    std::string &substring) {
  int idx = -1;
  for (int n = 0; n < lines.size(); ++n) {
    if (boost::contains(lines[n], substring)) {
      idx = n;
      break;
    }
  }
  return idx;
}

bool file_exists(const std::string &filename) {
  struct stat buffer;
  return (stat(filename.c_str(), &buffer) == 0);
}

}  // namespace test
}  // namespace cmdstan
#endif
