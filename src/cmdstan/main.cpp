#include <stan/services/command.hpp>
#include <boost/exception/diagnostic_information.hpp> 
#include <boost/exception_ptr.hpp> 

int main(int argc, const char* argv[]) {
  try {
    return stan::services::command<stan_model>(argc,argv);
  } catch (const std::exception& e) {
    std::cerr << std::endl << "Exception: " << e.what() << std::endl;
    std::cerr << "Diagnostic information: " << std::endl << boost::diagnostic_information(e) << std::endl;
    return -1;
  }
}
