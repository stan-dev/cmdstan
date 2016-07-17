#ifndef CMDSTAN_ARGUMENTS_UNVALUED_ARGUMENT_HPP
#define CMDSTAN_ARGUMENTS_UNVALUED_ARGUMENT_HPP

#include <cmdstan/arguments/argument.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace cmdstan {

  class unvalued_argument: public argument {
  public:
    unvalued_argument()
      : _is_present(false) {}

    void print(stan::interface_callbacks::writer::base_writer& w,
               const int depth,
               const std::string& prefix) {}

    void print_help(stan::interface_callbacks::writer::base_writer& w,
                    const int depth,
                    const bool recurse = false) {
      std::string indent(indent_width * depth, ' ');
      std::string subindent(indent_width, ' ');

      w(indent + _name);
      w(indent + subindent + _description);
      w();
    }

    bool parse_args(std::vector<std::string>& args,
                    stan::interface_callbacks::writer::base_writer& info,
                    stan::interface_callbacks::writer::base_writer& err,
                    bool& help_flag) {
      if (args.size() == 0)
        return true;

      if ((args.back() == "help") || (args.back() == "help-all")) {
        print_help(info, 0);
        help_flag |= true;
        args.clear();
        return true;
      }

      _is_present = true;
      return true;
    }

    bool is_present() {
      return _is_present;
    }

  protected:
    bool _is_present;
  };

}
#endif
