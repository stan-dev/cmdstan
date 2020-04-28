#include <cmdstan/command.hpp>
#include <stan/services/error_codes.hpp>

int main(int argc, const char *argv[]) {
  try {
    return cmdstan::command(argc, argv);
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return stan::services::error_codes::SOFTWARE;
  }
}
