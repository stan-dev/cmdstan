#ifndef CMDSTAN_ARGUMENTS_SINGLETON_ARGUMENT_HPP
#define CMDSTAN_ARGUMENTS_SINGLETON_ARGUMENT_HPP

#include <cmdstan/arguments/valued_argument.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace cmdstan {

namespace internal {
void from_string(std::string &src, double &dest) { dest = std::stod(src); }
void from_string(std::string &src, int &dest) { dest = std::stoi(src); }
void from_string(std::string &src, long long int &dest) {
  dest = std::stoll(src);
}
void from_string(std::string &src, unsigned int &dest) {
  dest = std::stoul(src);
}
void from_string(std::string &src, bool &dest) {
  if (src == "true" || src == "1") {
    dest = true;
  } else if (src == "false" || src == "0") {
    dest = false;
  } else {
    throw std::invalid_argument(std::string("invalid boolean value ") + src);
  }
}
void from_string(std::string &src, std::string &dest) { dest = src; }

std::string to_string(std::string &src) { return src; }

std::string to_string(double &src) {
  // better handling of precision than std::to_string
  std::stringstream ss;
  ss << src;
  return ss.str();
}

std::string to_string(bool &src) { return src ? "true" : "false"; }

template <typename T>
std::string to_string(T &src) {
  return std::to_string(src);
}
}  // namespace internal

template <typename T>
struct type_name {
  static std::string name() { return typeid(T).name(); }
};

// Specialize to something more readable
template <>
struct type_name<int> {
  static std::string name() { return "int"; }
};

template <>
struct type_name<unsigned int> {
  static std::string name() { return "unsigned int"; }
};

template <>
struct type_name<double> {
  static std::string name() { return "double"; }
};

template <>
struct type_name<bool> {
  static std::string name() { return "boolean"; }
};

template <>
struct type_name<std::string> {
  static std::string name() { return "string"; }
};

template <typename T>
class singleton_argument : public valued_argument {
 public:
  singleton_argument() : _validity("All") {
    _constrained = false;
    _name = "";
    _value_type = type_name<T>::name();
  }

  explicit singleton_argument(const std::string name) : _validity("All") {
    _name = name;
  }

  bool parse_args(std::vector<std::string> &args, stan::callbacks::writer &info,
                  stan::callbacks::writer &err, bool &help_flag) {
    if (args.size() == 0)
      return true;

    if ((args.back() == "help") || (args.back() == "help-all")) {
      print_help(info, 0);
      help_flag |= true;
      args.clear();
      return true;
    }

    std::string name;
    std::string value;
    split_arg(args.back(), name, value);

    if (_name == name) {
      args.pop_back();

      try {
        T proposed_value;
        internal::from_string(value, proposed_value);
        if (set_value(proposed_value)) {
          return true;
        }
      } catch (...) {
        // intentionally empty.
      }
      std::stringstream message;
      message << value << " is not a valid value for "
              << "\"" << _name << "\"";
      err(message.str());
      err(std::string(indent_width, ' ') + "Valid values:" + print_valid());

      args.clear();
      return false;
    }
    return true;
  }

  void find_arg(const std::string &name, const std::string &prefix,
                std::vector<std::string> &valid_paths) {
    if (name == _name) {
      valid_paths.push_back(prefix + _name + "=<" + _value_type + ">");
    }
  }

  T value() { return _value; }

  bool set_value(const T &value) {
    if (is_valid(value)) {
      _value = value;
      return true;
    }
    return false;
  }

  std::string print_value() { return internal::to_string(_value); }

  std::string print_valid() { return " " + _validity; }

  bool is_default() { return _value == _default_value; }

  virtual void print(stan::callbacks::structured_writer &j) {
    j.write(_name, _value);
  }

 protected:
  std::string _validity;
  virtual bool is_valid(T value) { return true; }

  T _value;
  T _default_value;

  bool _constrained;

  T _good_value;
  T _bad_value;
};

typedef singleton_argument<double> real_argument;
typedef singleton_argument<int> int_argument;
typedef singleton_argument<long long int> long_long_int_argument;
typedef singleton_argument<unsigned int> u_int_argument;
typedef singleton_argument<bool> bool_argument;
typedef singleton_argument<std::string> string_argument;
}  // namespace cmdstan
#endif
