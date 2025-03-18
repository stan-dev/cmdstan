#include <stdexcept>
#include <iostream>
#include <cerrno>

#include <cmdstan/command.hpp>
#include <stan/services/error_codes.hpp>

int main(int argc, const char *argv[]) {
  try {
    int err_code = cmdstan::command(argc, argv);
    if (err_code == 0)
      return cmdstan::return_codes::OK;
    else
      return cmdstan::return_codes::NOT_OK;
  } catch (std::system_error &e) {
    // system_error is thrown by std::ofstream but the
    // message is not always helpful, so we also print errno
    std::cerr << e.what() << ": " << std::strerror(errno) << std::endl;
    return cmdstan::return_codes::NOT_OK;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return cmdstan::return_codes::NOT_OK;
  }
}
