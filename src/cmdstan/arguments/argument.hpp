#ifndef CMDSTAN_ARGUMENTS_ARGUMENT_HPP
#define CMDSTAN_ARGUMENTS_ARGUMENT_HPP

#include <stan/callbacks/writer.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

namespace cmdstan {

  class argument {
  public:
    argument()
      : indent_width(2),
        help_width(20) { }

    explicit argument(const std::string& name)
      : _name(name),
        indent_width(2),
        help_width(20) { }

    virtual ~argument() { }

    std::string name() const {
      return _name;
    }

    std::string description() const {
      return _description;
    }

    virtual void print(stan::callbacks::writer& w,
                       const int depth,
                       const std::string& prefix) = 0;

    virtual void print_help(stan::callbacks::writer& w,
                            const int depth,
                            const bool recurse) = 0;

    virtual bool parse_args(std::vector<std::string>& args,
                            stan::callbacks::writer& info,
                            stan::callbacks::writer& err,
                            bool& help_flag) {
      return true;
    }

    virtual void probe_args(argument* base_arg,
                            stan::callbacks::writer& w) {}

    virtual void find_arg(const std::string& name,
                          const std::string& prefix,
                          std::vector<std::string>& valid_paths) {
      if (name == _name) {
        valid_paths.push_back(prefix + _name);
      }
    }

    static void split_arg(const std::string& arg,
                          std::string& name,
                          std::string& value) {
      size_t pos = arg.find('=');

      if (pos != std::string::npos) {
        name = arg.substr(0, pos);
        value = arg.substr(pos + 1, arg.size() - pos);
      } else {
        name = arg;
        value = "";
      }
    }

    virtual argument* arg(const std::string& name) {
      return 0;
    }

    int compute_indent(const int depth) {
      return indent_width * depth;
    }

  protected:
    std::string _name;
    std::string _description;

    int indent_width;
    int help_width;
  };

}
#endif
