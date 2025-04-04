#ifndef CMDSTAN_FILE_HPP
#define CMDSTAN_FILE_HPP

#include <boost/algorithm/string.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <memory>

namespace cmdstan {
namespace file {

#if defined(WIN32) || defined(_WIN32) \
    || defined(__WIN32) && !defined(__CYGWIN__)
constexpr char PATH_SEPARATOR = '\\';
#else
constexpr char PATH_SEPARATOR = '/';
#endif

/**
 * Distinguish between dot char '.' used as suffix sep
 * and relative filepaths '.' and '..'
 */
bool valid_dot_suffix(char prev, char current, char next) {
  if (current != '.') {
    return false;
  }
  if (prev == '\0' || next == '\0') {
    return false;
  }
  if (prev == '.' || next == '.') {
    return false;
  }
  if (prev == PATH_SEPARATOR || next == PATH_SEPARATOR) {
    return false;
  }
  return true;
}

/**
 * Find start of filename suffix, if any.
 * Start search from end of string, quit at first path separator.
 *
 * @return index of suffix separator '.' , or std::string::npos if not found.
 */
size_t find_dot_suffix(const std::string &input) {
  if (input.empty()) {
    return std::string::npos;
  }
  for (size_t i = input.size() - 1; i != 0; --i) {
    if (input[i] == PATH_SEPARATOR)
      return std::string::npos;
    char prev = i < input.size() - 1 ? input[i + 1] : '\0';
    char next = i > 0 ? input[i - 1] : '\0';
    if (valid_dot_suffix(next, input[i], prev)) {
      return i;
    }
  }
  return std::string::npos;
}

/**
 * Get suffix
 *
 * @param filename
 * @return suffix
 */
std::string get_suffix(const std::string &name) {
  if (name.empty())
    return "";
  std::string filename = name;
  size_t idx = name.find_last_of(PATH_SEPARATOR);
  if (idx < name.size())
    filename = name.substr(idx);
  idx = find_dot_suffix(filename);
  if (idx > filename.size())
    return std::string();
  else
    return filename.substr(idx);
}

/**
 * Split name on last "." with at least one following char.
 * If suffix is good, return pair base, suffix including initial '.'.
 * Else return pair base, empty string.
 *
 * @param filename - name to split
 * @return pair of strings {base, suffix}
 */
std::pair<std::string, std::string> get_basename_suffix(
    const std::string &name) {
  std::string base;
  std::string suffix;
  if (!name.empty()) {
    suffix = get_suffix(name);
    if (suffix.size() > 1) {
      base = name.substr(0, name.size() - suffix.size());
    } else {
      base = name;
      suffix = "";
    }
  }
  return {base, suffix};
}

std::vector<std::string> split_on_comma(const std::string &input) {
  std::vector<std::string> result;
  boost::algorithm::split(result, input, boost::is_any_of(","),
                          boost::token_compress_on);
  return result;
}

/**
 * Check if two file paths are the same file.
 * @note This function only handles very basic access patterns.
 *  It will miss cases such as
 *  path1: ./a/b/../b/file.txt
 *  path1: ./a/b/file.txt
 * @param path1 first file path
 * @param path2 second file path
 * @return true if paths are the same file, false otherwise
 */
bool check_approx_same_file(const std::string &path1,
                            const std::string &path2) {
  // if path1 has a comma in it, check all of them
  // if any match, return true
  if (path1.find(',') != std::string::npos) {
    for (const auto &name : split_on_comma(path1)) {
      if (check_approx_same_file(name, path2)) {
        return true;
      }
    }
    return false;
  }

  // same for path2
  if (path2.find(',') != std::string::npos) {
    for (const auto &name : split_on_comma(path2)) {
      if (check_approx_same_file(path1, name)) {
        return true;
      }
    }
    return false;
  }

  const auto path1_size = path1.size();
  const auto path2_size = path2.size();
  if (path1.empty() || path2.empty()) {
    return false;
  }

  for (int path1_i = path1_size - 1, path2_i = path2_size - 1;
       path1_i >= 0 && path2_i >= 0; --path1_i, --path2_i) {
    if (path1[path1_i] != path2[path2_i]) {
      return false;
    }
    // Check for path seperator, then look ahead to see
    //  if we have ./ or ../
    int tick_1 = 0;
    int tick_2 = 0;
    if (path1[path1_i] == PATH_SEPARATOR) {
      if (path1_i > 0 && path1[path1_i - 1] == '.') {
        tick_1 = 1;
      } else if (path1_i > 1 && path1[path1_i - 1] == '.'
                 && path1[path1_i - 2] == '.') {
        tick_1 = 2;
      }
    }
    if (path2[path2_i] == PATH_SEPARATOR) {
      if (path2_i > 0 && path2[path2_i - 1] == '.') {
        tick_2 = 1;
      } else if (path2_i > 1 && path2[path2_i - 1] == '.'
                 && path2[path2_i - 2] == '.') {
        tick_2 = 2;
      }
    }
    // If ./ or ../ then skip it
    path1_i -= tick_1;
    path2_i -= tick_2;
  }
  return true;
}

/**
 * Check that output filename isn't a directory name
 * or relative dir path.
 * Throws exception if output filename is invalid.
 *
 * @param fname candidate output filename
 */
void validate_output_filename(const std::string &fname) {
  // if a , is present, check all values
  if (fname.find(',') != std::string::npos) {
    for (const auto &name : split_on_comma(fname)) {
      validate_output_filename(name);
    }
    return;
  }

  std::string sep = std::string(1, cmdstan::file::PATH_SEPARATOR);
  if (!fname.empty()
      && (fname[fname.size() - 1] == PATH_SEPARATOR
          || boost::algorithm::ends_with(fname, "..")
          || boost::algorithm::ends_with(fname, sep + "."))) {
    std::stringstream msg;
    msg << "Ill-formed output filename " << fname << std::endl;
    throw std::invalid_argument(msg.str());
  }
}

/**
 * Construct output file names given template filename,
 * adding tags and numbers as needed for per-chain outputs.
 * Output file types are either CSV or JSON.
 * Template filenames may already contain suffix ".csv" or "json".
 *
 * @param filename output or diagnostic filename, user-specified or default.
 * @param tag distinguishing tag
 * @param type suffix string corresponding to types CSV, JSON
 * @param num_chains number of names to return
 * @param id numbering offset
 */
std::vector<std::string> make_filenames(const std::string &filename,
                                        const std::string &tag,
                                        const std::string &type,
                                        unsigned int num_chains,
                                        unsigned int id) {
  std::vector<std::string> names(num_chains);

  // if a ',' is present, we assume the user fully specified the names
  if (filename.find(',') != std::string::npos) {
    std::vector<std::string> filenames = split_on_comma(filename);
    if (filenames.size() != num_chains) {
      std::stringstream msg;
      msg << "Number of filenames does not match number of chains: got "
             "comma-separated list '"
          << filename << "' of length " << filenames.size() << " but expected "
          << num_chains << " names" << std::endl;
      throw std::invalid_argument(msg.str());
    }

    std::transform(filenames.cbegin(), filenames.cend(), names.begin(),
                   [&tag, &type](const std::string &name) {
                     auto [base_name, sfx] = get_basename_suffix(name);
                     if ((!type.empty() && type != ".csv") || sfx.empty()) {
                       sfx = type;
                     }
                     // TODO: in most cases tag is empty, it would be nice if it
                     // was never used for maximum user control
                     return base_name + tag + sfx;
                   });
  } else {
    // otherwise, this is a template which gets edited like output.csv ->
    // output_1.csv
    auto [base_name, sfx] = get_basename_suffix(filename);

    // first condition here is legacy -- we used to be very lax
    // about file names, but with things like json outputs
    // we need to be stricter to avoid collisions, so we only
    // allow laxity on the suffix for intended-to-be csv files
    if ((!type.empty() && type != ".csv") || sfx.empty()) {
      sfx = type;
    }

    auto name_iterator = [num_chains, id](auto i) {
      if (num_chains == 1) {
        return std::string("");
      } else {
        return std::string("_" + std::to_string(i + id));
      }
    };
    for (int i = 0; i < num_chains; ++i) {
      names[i] = base_name + tag + name_iterator(i) + sfx;
    }
  }

  return names;
}

/**
 * Opens input stream for file.
 * Throws exception if stream cannot be opened.
 *
 * @param fname name of file which exists and has read perms.
 * @return input stream
 */
std::ifstream safe_open(const std::string &fname) {
  std::ifstream stream(fname.c_str());
  if (fname != "" && (stream.rdstate() & std::ifstream::failbit)) {
    std::stringstream msg;
    msg << "Cannot open specified file, \"" << fname << "\"" << std::endl;
    throw std::invalid_argument(msg.str());
  }
  return stream;
}

/**
 * Opens input stream for file and sets exception flags.
 *
 * @param fname name of file which exists and has read perms.
 * @param sig_figs number of digits to print
 * @return Unique pointer to the output stream
 */
std::unique_ptr<std::ofstream> safe_create(const std::string &fname,
                                           int sig_figs) {
  auto ofs = std::make_unique<std::ofstream>(fname.c_str());
  ofs->exceptions(std::ofstream::badbit | std::ofstream::failbit);
  if (sig_figs > -1) {
    ofs->precision(sig_figs);
  }
  return ofs;
}

}  // namespace file
}  // namespace cmdstan

#endif
