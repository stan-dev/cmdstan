#include <gtest/gtest.h>
#include <stan/services/error_codes.hpp>
#include <cmdstan/arguments/argument_probe.hpp>
#include <cmdstan/arguments/arg_method.hpp>
#include <cmdstan/arguments/arg_id.hpp>
#include <cmdstan/arguments/arg_data.hpp>
#include <cmdstan/arguments/arg_init.hpp>
#include <cmdstan/arguments/arg_random.hpp>
#include <cmdstan/arguments/arg_output.hpp>
#include <stan/callbacks/stream_writer.hpp>
#include <test/utility.hpp>

#include <vector>
#include <sstream>

using cmdstan::test::convert_model_path;
using cmdstan::test::multiple_command_separator;
using cmdstan::test::run_command;
using cmdstan::test::run_command_output;

void clean_line(std::string& line) {
  if (line.find("(Default)") != std::string::npos)
    line = line.substr(0, line.find("(Default)"));
  line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
}

void remove_duplicates(std::string& argument) {
  unsigned int cursor = argument.find("=");
  while (cursor < argument.length()) {
    unsigned int end = argument.find(" ", cursor) - 1;
    std::string value = " " + argument.substr(cursor + 1, end - cursor);

    if (value.length() == 1) ++end;
    else if (argument.find(value, end) != std::string::npos)
      argument.erase(argument.find(value, end), value.length());
    cursor = argument.find("=", end);
  }
}

class StanGmArgumentsConfiguration : public testing::Test {
public:
  static void SetUpTestCase() {
    std::vector<std::string> model_path;
    model_path.push_back("..");
    model_path.push_back("src");
    model_path.push_back("test");
    model_path.push_back("test-models");
    model_path.push_back("test_model");

    command += "cd test ";
    command += multiple_command_separator();
    command += " ";
    command += convert_model_path(model_path);
  }

  static void TearDownTestCase() {
  }

  static std::string command;
};
std::string StanGmArgumentsConfiguration::command = "";

TEST_F(StanGmArgumentsConfiguration, TestMethod) {
  // Prepare arguments
  std::stringstream s;
  stan::callbacks::stream_writer w(s);

  std::vector<cmdstan::argument*> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_method());
  
  cmdstan::argument_probe probe(valid_arguments);
  probe.probe_args(w);

  // Check argument consistency
  bool expected_success = false;
  
  std::string l1;
  std::stringstream expected_output;
  std::stringstream output;
  
  while (s.good()) {
    std::getline(s, l1);
    if (!s.good()) 
      continue;
    
    if (l1 == "good") 
      expected_success = true;
    else if (l1 == "bad") 
      expected_success = false;
    else if (l1 != "") 
      expected_output << l1 << std::endl;
    else {
      int n_output = 0;
      
      std::string l2;
      std::string argument("");
      
      while (expected_output.good()) {
        std::getline(expected_output, l2);
        if (!expected_output.good()) 
          continue;
        clean_line(l2);
        argument += " " + l2;
        ++n_output;
      }
      
      if (argument.length() == 0) 
        continue;
      
      remove_duplicates(argument);

      SCOPED_TRACE(command + argument);
      
      run_command_output out = run_command(command + argument);
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);

      if (expected_success == false) {
        unsigned int c1 = out.output.find("is not");
        out.output.erase(0, c1);

        unsigned int c2 = out.output.find(" \"");

        unsigned int c3 = out.output.find("Failed to parse");

        if (c3 != c2)
          out.output.replace(c2, c3 - c2, "\n");
        
        expected_output.str(std::string());

        expected_output << "is not a valid value for" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
        
        EXPECT_EQ(int(stan::services::error_codes::USAGE), out.err_code);
        
      } else {
        EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code) 
          << "command: " << out.command;
      }
      
      output.clear();
      output.seekg(std::ios_base::beg);
      output.str(out.output);
      std::string actual_line;

      for (int i = 0; i < n_output; ++i) {
        std::string expected_line;
        std::getline(expected_output, expected_line);
        
        std::getline(output, actual_line);
        
        EXPECT_EQ(expected_line, actual_line);
      }
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str(std::string());
    }
  }
  
  for (size_t i = 0; i < valid_arguments.size(); ++i)
    delete valid_arguments.at(i);
}

TEST_F(StanGmArgumentsConfiguration, TestIdWithMethod) {
  
  // Prepare arguments
  std::stringstream method_output;
  stan::callbacks::stream_writer method_writer(method_output);
  cmdstan::arg_method method;
  method.print(method_writer, 0, "");
  
  std::string l0;
  std::string method_argument("");
  int n_method_output = 0;
  
  while (method_output.good()) {
    std::getline(method_output, l0);
    if (!method_output.good()) continue;
    clean_line(l0);
    method_argument += " " + l0;
    ++n_method_output;
  }
      
  remove_duplicates(method_argument);

  method_output.clear();
  method_output.seekg(std::ios_base::beg);

  std::stringstream s;
  stan::callbacks::stream_writer w(s);
  std::vector<cmdstan::argument*> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_id());
  
  cmdstan::argument_probe probe(valid_arguments);
  probe.probe_args(w);
  
  // Check argument consistency
  bool expected_success = false;
  
  std::string l1;
  std::stringstream expected_output;
  std::stringstream output;
  
  while (s.good()) {
    
    std::getline(s, l1);
    if (!s.good()) continue;
    
    if      (l1 == "good") expected_success = true;
    else if (l1 == "bad") expected_success = false;
    else if (l1 != "") expected_output << l1 << std::endl;
    else {
      
      int n_output = 0;
      
      std::string l2;
      std::string argument("");
      
      while (expected_output.good()) {
        std::getline(expected_output, l2);
        if (!expected_output.good()) continue;
        clean_line(l2);
        argument += " " + l2;
        ++n_output;
      }
      
      if (argument.length() == 0) continue;
      remove_duplicates(argument);
      argument = method_argument + argument;
      
      run_command_output out = run_command(command + argument);
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str( method_output.str() + expected_output.str() );
      
      if (expected_success == false) {
        
        unsigned int c1 = out.output.find("is not");
        out.output.erase(0, c1);
        unsigned int c2 = out.output.find(" \"");
        unsigned int c3 = out.output.find("Failed to parse");
        out.output.replace(c2, c3 - c2, "\n");
        
        expected_output.str(std::string());
        expected_output << "is not a valid value for" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
        
      } else {
        EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code) 
          << "command: " << out.command;
      }
    
      output.str(out.output);
      std::string actual_line;
      
      for (int i = 0; i < n_output; ++i) {
        std::string expected_line;
        std::getline(expected_output, expected_line);
        
        std::getline(output, actual_line);
        
        EXPECT_EQ(expected_line, actual_line);
      }
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str(std::string());
      
    }
    
  }
  
  for (size_t i = 0; i < valid_arguments.size(); ++i)
    delete valid_arguments.at(i);

}


TEST_F(StanGmArgumentsConfiguration, TestIdWithoutMethod) {
  
  // Prepare arguments
  std::stringstream s;
  stan::callbacks::stream_writer w(s);
  
  std::vector<cmdstan::argument*> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_id());
  
  cmdstan::argument_probe probe(valid_arguments);
  probe.probe_args(w);
  
  // Check argument consistency
  bool expected_success = false;
  
  std::string l1;
  std::stringstream expected_output;
  std::stringstream output;
  
  while (s.good()) {
    
    std::getline(s, l1);
    if (!s.good()) continue;
    
    if      (l1 == "good") expected_success = true;
    else if (l1 == "bad") expected_success = false;
    else if (l1 != "") expected_output << l1 << std::endl;
    else {
      
      int n_output = 0;
      
      std::string l2;
      std::string argument("");
      
      while (expected_output.good()) {
        std::getline(expected_output, l2);
        if (!expected_output.good()) continue;
        clean_line(l2);
        argument += " " + l2;
        ++n_output;
      }
      
      if (argument.length() == 0) continue;
      
      remove_duplicates(argument);
      
      run_command_output out = run_command(command + argument);
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      
      if (expected_success == false) {
        
        unsigned int c1 = out.output.find("is not");
        out.output.erase(0, c1);
        unsigned int c2 = out.output.find(" \"");
        unsigned int c3 = out.output.find("Failed to parse");
        out.output.replace(c2, c3 - c2, "\n");
        
        expected_output.str(std::string());
        expected_output << "is not a valid value for" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
        
      }
      else {
        expected_output.str(std::string());
        expected_output << "A method must be specified!" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
      }
    
      output.str(out.output);
      std::string actual_line;
      
      for (int i = 0; i < n_output; ++i) {
        std::string expected_line;
        std::getline(expected_output, expected_line);
        
        std::getline(output, actual_line);
        
        EXPECT_EQ(expected_line, actual_line);
      }
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str(std::string());
      
    }
    
  }
  
  for (size_t i = 0; i < valid_arguments.size(); ++i)
    delete valid_arguments.at(i);
  
}

TEST_F(StanGmArgumentsConfiguration, TestDataWithMethod) {
  
  // Prepare arguments
  std::stringstream method_output;
  stan::callbacks::stream_writer method_writer(method_output);
  cmdstan::arg_method method;
  method.print(method_writer, 0, "");
  
  std::string l0;
  std::string method_argument("");
  int n_method_output = 0;

  while (method_output.good()) {
    std::getline(method_output, l0);
    if (!method_output.good()) continue;
    clean_line(l0);
    method_argument += " " + l0;
    ++n_method_output;
  }
  
  remove_duplicates(method_argument);
  
  method_output.clear();
  method_output.seekg(std::ios_base::beg);
  
  std::stringstream s;
  stan::callbacks::stream_writer w(s);
  std::vector<cmdstan::argument*> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_data());
  
  cmdstan::argument_probe probe(valid_arguments);
  probe.probe_args(w);
  
  // Check argument consistency
  bool expected_success = false;
  
  std::string l1;
  std::stringstream expected_output;
  std::stringstream output;
  
  while (s.good()) {
    std::getline(s, l1);
    if (!s.good()) continue;

    if (l1 == "good") expected_success = true;
    else if (l1 == "bad") expected_success = false;
    else if (l1 != "") expected_output << l1 << std::endl;
    else {
      int n_output = 0;
      
      std::string l2;
      std::string argument("");
      
      while (expected_output.good()) {
        std::getline(expected_output, l2);
        if (!expected_output.good()) continue;
        clean_line(l2);
        // replace "file=good" with "file=<real file>"
        if (l2 == "file=good")
          l2 = "file=../src/test/test-models/test_model.init.R";
        argument += " " + l2;
        ++n_output;
      }
      
      if (argument.length() == 0) continue;
      remove_duplicates(argument);
      argument = method_argument + argument;
      
      run_command_output out = run_command(command + argument);

      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str( method_output.str() + expected_output.str() );
      
      if (expected_success == false) {
        
        unsigned int c1 = out.output.find("is not");
        out.output.erase(0, c1);
        unsigned int c2 = out.output.find(" \"");
        unsigned int c3 = out.output.find("Failed to parse");
        out.output.replace(c2, c3 - c2, "\n");
        
        expected_output.str(std::string());
        expected_output << "is not a valid value for" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
        
      } else {
        EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code) 
          << "command: " << out.command;
      }
      
      output.str(out.output);
      std::string actual_line;
      
      for (int i = 0; i < n_output; ++i) {
        std::string expected_line;
        std::getline(expected_output, expected_line);
        
        std::getline(output, actual_line);
        
        EXPECT_EQ(expected_line, actual_line)
          << "expected-success = " << expected_success << std::endl
          << "l2 = " << l2 << std::endl
          << "l1 = " << l1 << std::endl;
      }
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str(std::string());
      
    }
    
  }
  
  for (size_t i = 0; i < valid_arguments.size(); ++i)
    delete valid_arguments.at(i);
  
}


TEST_F(StanGmArgumentsConfiguration, TestDataWithoutMethod) {
  
  // Prepare arguments
  std::stringstream s;
  stan::callbacks::stream_writer w(s);
  
  std::vector<cmdstan::argument*> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_data());
  
  cmdstan::argument_probe probe(valid_arguments);
  probe.probe_args(w);
  
  // Check argument consistency
  bool expected_success = false;
  
  std::string l1;
  std::stringstream expected_output;
  std::stringstream output;
  
  while (s.good()) {
    
    std::getline(s, l1);
    if (!s.good()) continue;
    
    if      (l1 == "good") expected_success = true;
    else if (l1 == "bad") expected_success = false;
    else if (l1 != "") expected_output << l1 << std::endl;
    else {
      
      int n_output = 0;
      
      std::string l2;
      std::string argument("");
      
      while (expected_output.good()) {
        std::getline(expected_output, l2);
        if (!expected_output.good()) continue;
        clean_line(l2);
        argument += " " + l2;
        ++n_output;
      }
      
      if (argument.length() == 0) continue;
      
      remove_duplicates(argument);

      run_command_output out = run_command(command + argument);
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      
      if (expected_success == false) {
        
        unsigned int c1 = out.output.find("is not");
        out.output.erase(0, c1);
        unsigned int c2 = out.output.find(" \"");
        unsigned int c3 = out.output.find("Failed to parse");
        out.output.replace(c2, c3 - c2, "\n");
        
        expected_output.str(std::string());
        expected_output << "is not a valid value for" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
        
      } else {
        expected_output.str(std::string());
        expected_output << "A method must be specified!" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
      }
      
      output.str(out.output);
      std::string actual_line;
      
      for (int i = 0; i < n_output; ++i) {
        std::string expected_line;
        std::getline(expected_output, expected_line);
        
        std::getline(output, actual_line);
        
        EXPECT_EQ(expected_line, actual_line);
      }
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str(std::string());
      
    }
    
  }
  
  for (size_t i = 0; i < valid_arguments.size(); ++i)
    delete valid_arguments.at(i);
  
}

TEST_F(StanGmArgumentsConfiguration, TestInitWithMethod) {
  
  // Prepare arguments
  std::stringstream method_output;
  stan::callbacks::stream_writer method_writer(method_output);
  cmdstan::arg_method method;
  method.print(method_writer, 0, "");
  
  std::string l0;
  std::string method_argument("");
  int n_method_output = 0;
  
  while (method_output.good()) {
    std::getline(method_output, l0);
    if (!method_output.good()) continue;
    clean_line(l0);
    method_argument += " " + l0;
    ++n_method_output;
  }
  
  remove_duplicates(method_argument);
  
  method_output.clear();
  method_output.seekg(std::ios_base::beg);
  
  std::stringstream s;
  stan::callbacks::stream_writer w(s);
  std::vector<cmdstan::argument*> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_init());
  
  cmdstan::argument_probe probe(valid_arguments);
  probe.probe_args(w);
  // Check argument consistency
  bool expected_success = false;
  
  std::string l1;
  std::stringstream expected_output;
  std::stringstream output;
  
  while (s.good()) {
    
    std::getline(s, l1);
    if (!s.good()) continue;
    
    if      (l1 == "good") expected_success = true;
    else if (l1 == "bad") expected_success = false;
    else if (l1 != "") expected_output << l1 << std::endl;
    else {
      
      int n_output = 0;
      
      std::string l2;
      std::string argument("");
      
      while (expected_output.good()) {
        std::getline(expected_output, l2);
        if (!expected_output.good()) continue;
        clean_line(l2);
        argument += " " + l2;
        ++n_output;
      }
      
      if (argument.length() == 0) continue;
      remove_duplicates(argument);
      argument = method_argument + argument;
      run_command_output out = run_command(command + argument);
      EXPECT_FALSE(out.hasError) 
        << "command: " << out.command << std::endl;

      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str( method_output.str() + expected_output.str() );
      
      if (expected_success == false) {
        
        unsigned int c1 = out.output.find("is not");
        out.output.erase(0, c1);
        unsigned int c2 = out.output.find(" \"");
        unsigned int c3 = out.output.find("Failed to parse");
        out.output.replace(c2, c3 - c2, "\n");
        
        expected_output.str(std::string());
        expected_output << "is not a valid value for" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
        
      } else {
        EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code) 
          << "command: " << out.command;
      }
      
      output.str(out.output);
      std::string actual_line;
      
      for (int i = 0; i < n_output; ++i) {
        std::string expected_line;
        std::getline(expected_output, expected_line);
        
        std::getline(output, actual_line);
        
        EXPECT_EQ(expected_line, actual_line);
      }
      
    }
      
  }
  
  for (size_t i = 0; i < valid_arguments.size(); ++i)
    delete valid_arguments.at(i);
  
}


TEST_F(StanGmArgumentsConfiguration, TestInitWithoutMethod) {
  
  // Prepare arguments
  std::stringstream s;
  stan::callbacks::stream_writer w(s);
  
  std::vector<cmdstan::argument*> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_init());
  
  cmdstan::argument_probe probe(valid_arguments);
  probe.probe_args(w);
  
  // Check argument consistency
  bool expected_success = false;
  
  std::string l1;
  std::stringstream expected_output;
  std::stringstream output;
  
  while (s.good()) {
    
    std::getline(s, l1);
    if (!s.good()) continue;
    
    if      (l1 == "good") expected_success = true;
    else if (l1 == "bad") expected_success = false;
    else if (l1 != "") expected_output << l1 << std::endl;
    else {
      
      int n_output = 0;
      
      std::string l2;
      std::string argument("");
      
      while (expected_output.good()) {
        std::getline(expected_output, l2);
        if (!expected_output.good()) continue;
        clean_line(l2);
        argument += " " + l2;
        ++n_output;
      }
      
      if (argument.length() == 0) continue;
      
      remove_duplicates(argument);
      
      run_command_output out = run_command(command + argument);
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      
      if (expected_success == false) {
        
        unsigned int c1 = out.output.find("is not");
        out.output.erase(0, c1);
        unsigned int c2 = out.output.find(" \"");
        unsigned int c3 = out.output.find("Failed to parse");
        out.output.replace(c2, c3 - c2, "\n");
        
        expected_output.str(std::string());
        expected_output << "is not a valid value for" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
        
      }
      else {
        expected_output.str(std::string());
        expected_output << "A method must be specified!" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
      }
      
      output.str(out.output);
      std::string actual_line;
    
      for (int i = 0; i < n_output; ++i) {
        std::string expected_line;
        std::getline(expected_output, expected_line);
        
        std::getline(output, actual_line);
        
        EXPECT_EQ(expected_line, actual_line);
      }
      
    }
    
  }
  
  for (size_t i = 0; i < valid_arguments.size(); ++i)
    delete valid_arguments.at(i);
  
}

TEST_F(StanGmArgumentsConfiguration, TestRandomWithMethod) {
  
  // Prepare arguments
  std::stringstream method_output;
  stan::callbacks::stream_writer method_writer(method_output);
  cmdstan::arg_method method;
  method.print(method_writer, 0, "");
  
  std::string l0;
  std::string method_argument("");
  int n_method_output = 0;
  
  while (method_output.good()) {
    std::getline(method_output, l0);
    if (!method_output.good()) continue;
    clean_line(l0);
    method_argument += " " + l0;
    ++n_method_output;
  }
  
  remove_duplicates(method_argument);
  
  method_output.clear();
  method_output.seekg(std::ios_base::beg);
  
  std::stringstream s;
  stan::callbacks::stream_writer w(s);
  std::vector<cmdstan::argument*> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_random());
  
  cmdstan::argument_probe probe(valid_arguments);
  probe.probe_args(w);
  
  // Check argument consistency
  bool expected_success = false;
  
  std::string l1;
  std::stringstream expected_output;
  std::stringstream output;
  
  while (s.good()) {
    
    std::getline(s, l1);
    if (!s.good()) continue;
    
    if      (l1 == "good") expected_success = true;
    else if (l1 == "bad") expected_success = false;
    else if (l1 != "") expected_output << l1 << std::endl;
    else {
      
      int n_output = 0;
      
      std::string l2;
      std::string argument("");
      
      while (expected_output.good()) {
        std::getline(expected_output, l2);
        if (!expected_output.good()) continue;
        clean_line(l2);
        argument += " " + l2;
        ++n_output;
      }
      
      if (argument.length() == 0) continue;
      remove_duplicates(argument);
      argument = method_argument + argument;
      
      run_command_output out = run_command(command + argument);
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str( method_output.str() + expected_output.str() );
      
      if (expected_success == false) {
        
        unsigned int c1 = out.output.find("is not");
        out.output.erase(0, c1);
        unsigned int c2 = out.output.find(" \"");
        unsigned int c3 = out.output.find("Failed to parse");
        out.output.replace(c2, c3 - c2, "\n");
        
        expected_output.str(std::string());
        expected_output << "is not a valid value for" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
        
      } else {
        EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code) 
          << "command: " << out.command;
      }
      
      output.str(out.output);
      std::string actual_line;
      
      for (int i = 0; i < n_output; ++i) {
        std::string expected_line;
        std::getline(expected_output, expected_line);
        
        std::getline(output, actual_line);
        
        EXPECT_EQ(expected_line, actual_line);
      }
      
    }
    
  }
  
  for (size_t i = 0; i < valid_arguments.size(); ++i)
    delete valid_arguments.at(i);
  
}


TEST_F(StanGmArgumentsConfiguration, TestRandomWithoutMethod) {
  
  // Prepare arguments
  std::stringstream s;
  stan::callbacks::stream_writer w(s);
  
  std::vector<cmdstan::argument*> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_random());
  
  cmdstan::argument_probe probe(valid_arguments);
  probe.probe_args(w);
  
  // Check argument consistency
  bool expected_success = false;
  
  std::string l1;
  std::stringstream expected_output;
  std::stringstream output;
  
  while (s.good()) {
    
    std::getline(s, l1);
    if (!s.good()) continue;
    
    if      (l1 == "good") expected_success = true;
    else if (l1 == "bad") expected_success = false;
    else if (l1 != "") expected_output << l1 << std::endl;
    else {
      
      int n_output = 0;
      
      std::string l2;
      std::string argument("");
      
      while (expected_output.good()) {
        std::getline(expected_output, l2);
        if (!expected_output.good()) continue;
        clean_line(l2);
        argument += " " + l2;
        ++n_output;
      }
      
      if (argument.length() == 0) continue;
      
      remove_duplicates(argument);
      
      run_command_output out = run_command(command + argument);
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      
      if (expected_success == false) {
        
        unsigned int c1 = out.output.find("is not");
        out.output.erase(0, c1);
        unsigned int c2 = out.output.find(" \"");
        unsigned int c3 = out.output.find("Failed to parse");
        out.output.replace(c2, c3 - c2, "\n");
        
        expected_output.str(std::string());
        expected_output << "is not a valid value for" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
        
      }
      else {
        expected_output.str(std::string());
        expected_output << "A method must be specified!" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
      }
      
      output.str(out.output);
      std::string actual_line;
      
      for (int i = 0; i < n_output; ++i) {
        std::string expected_line;
        std::getline(expected_output, expected_line);
        
        std::getline(output, actual_line);
        
        EXPECT_EQ(expected_line, actual_line);
      }
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str(std::string());
      
    }
    
  }
  
  for (size_t i = 0; i < valid_arguments.size(); ++i)
    delete valid_arguments.at(i);
  
}

TEST_F(StanGmArgumentsConfiguration, TestOutputWithMethod) {
  
  // Prepare arguments
  std::stringstream method_output;
  stan::callbacks::stream_writer method_writer(method_output);  
  cmdstan::arg_method method;
  method.print(method_writer, 0, "");
  
  std::string l0;
  std::string method_argument("");
  int n_method_output = 0;
  
  while (method_output.good()) {
    std::getline(method_output, l0);
    if (!method_output.good()) continue;
    clean_line(l0);
    method_argument += " " + l0;
    ++n_method_output;
  }
  
  remove_duplicates(method_argument);
  
  method_output.clear();
  method_output.seekg(std::ios_base::beg);
  
  std::stringstream s;
  stan::callbacks::stream_writer w(s);
  std::vector<cmdstan::argument*> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_output());
  
  cmdstan::argument_probe probe(valid_arguments);
  probe.probe_args(w);
  
  // Check argument consistency
  bool expected_success = false;
  
  std::string l1;
  std::stringstream expected_output;
  std::stringstream output;
  
  while (s.good()) {
    
    std::getline(s, l1);
    if (!s.good()) continue;
    
    if      (l1 == "good") expected_success = true;
    else if (l1 == "bad") expected_success = false;
    else if (l1 != "") expected_output << l1 << std::endl;
    else {
      
      int n_output = 0;
      
      std::string l2;
      std::string argument("");

      while (expected_output.good()) {
        std::getline(expected_output, l2);
        if (!expected_output.good()) continue;
        clean_line(l2);
        argument += " " + l2;
        ++n_output;
      }
      
      if (argument.length() == 0) continue;
      remove_duplicates(argument);
      argument = method_argument + argument;
      
      run_command_output out = run_command(command + argument);
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str( method_output.str() + expected_output.str() );
      
      if (expected_success == false) {
        
        unsigned int c1 = out.output.find("is not");
        out.output.erase(0, c1);
        unsigned int c2 = out.output.find(" \"");
        unsigned int c3 = out.output.find("Failed to parse");
        out.output.replace(c2, c3 - c2, "\n");
        
        expected_output.str(std::string());
        expected_output << "is not a valid value for" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
        
      } else {
        EXPECT_EQ(int(stan::services::error_codes::OK), out.err_code) 
          << "command: " << out.command;
      }
      
      output.str(out.output);
      std::string actual_line;
      
      for (int i = 0; i < n_output; ++i) {
        std::string expected_line;
        std::getline(expected_output, expected_line);
        
        std::getline(output, actual_line);
        
        EXPECT_EQ(expected_line, actual_line);
      }
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str(std::string());
      
    }
    
  }
  
  for (size_t i = 0; i < valid_arguments.size(); ++i)
    delete valid_arguments.at(i);
  
}


TEST_F(StanGmArgumentsConfiguration, TestOutputWithoutMethod) {
  
  // Prepare arguments
  std::stringstream s;
  stan::callbacks::stream_writer w(s);
  
  std::vector<cmdstan::argument*> valid_arguments;
  valid_arguments.push_back(new cmdstan::arg_output());
  
  cmdstan::argument_probe probe(valid_arguments);
  probe.probe_args(w);
  
  // Check argument consistency
  bool expected_success = false;
  
  std::string l1;
  std::stringstream expected_output;
  std::stringstream output;
  
  while (s.good()) {
    
    std::getline(s, l1);
    if (!s.good()) continue;
    
    if      (l1 == "good") expected_success = true;
    else if (l1 == "bad") expected_success = false;
    else if (l1 != "") expected_output << l1 << std::endl;
    else {
      
      int n_output = 0;
      
      std::string l2;
      std::string argument("");
      
      while (expected_output.good()) {
        std::getline(expected_output, l2);
        if (!expected_output.good()) continue;
        clean_line(l2);
        argument += " " + l2;
        ++n_output;
      }
      
      if (argument.length() == 0) continue;
      
      remove_duplicates(argument);
      
      run_command_output out = run_command(command + argument);
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      
      if (expected_success == false) {
        
        unsigned int c1 = out.output.find("is not");
        out.output.erase(0, c1);
        unsigned int c2 = out.output.find(" \"");
        unsigned int c3 = out.output.find("Failed to parse");
        out.output.replace(c2, c3 - c2, "\n");
        
        expected_output.str(std::string());
        expected_output << "is not a valid value for" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
        
      }
      else {
        expected_output.str(std::string());
        expected_output << "A method must be specified!" << std::endl;
        expected_output << "Failed to parse arguments, terminating Stan" << std::endl;
        n_output = 2;
      }
      
      output.str(out.output);
      std::string actual_line;
      
      for (int i = 0; i < n_output; ++i) {
        std::string expected_line;
        std::getline(expected_output, expected_line);
        
        std::getline(output, actual_line);
        
        EXPECT_EQ(expected_line, actual_line);
      }
      
      expected_output.clear();
      expected_output.seekg(std::ios_base::beg);
      expected_output.str(std::string());
      
    }
    
  }
  
  for (size_t i = 0; i < valid_arguments.size(); ++i)
    delete valid_arguments.at(i);
  
}
