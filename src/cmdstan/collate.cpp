#include <cmdstan/write_draw.hpp>
#include <cmdstan/write_header.hpp>
#include <cmdstan/write_model.hpp>
#include <cmdstan/write_stan.hpp>
#include <stan/callbacks/stream_writer.hpp>
#include <stan/callbacks/writer.hpp>
#include <stan/io/stan_csv_reader.hpp>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <ios>

void collate_usage() {
  std::cout << "USAGE:  collate <options> <filename 1> [<filename 2> ... <filename N>]"
            << std::endl
            << std::endl;

  std::cout << "OPTIONS:" << std::endl << std::endl;
  std::cout << "  --collate_csv_file=<filename>\tWrite output as csv file "
            << std::endl
            << std::endl;
}

/**
 * The Stan collate function.
 *
 * @param argc Number of arguments
 * @param argv Arguments
 * 
 * @return 0 for success, 
 *         non-zero otherwise
 */
int main(int argc, const char* argv[]) {
  
  if (argc == 1) {
    collate_usage();
    return 0;
  }

  std::string collate_filename = "combined_output.csv";
  
  // Parse arguments specifying filenames
  std::ifstream ifstream;
  std::vector<std::string> filenames;
  for (int i = 1; i < argc; i++) {

    if (std::string(argv[i]).find("--collate_csv_file=") != std::string::npos) {
      collate_filename = std::string(argv[i]).substr(19).c_str();
      continue;
    }
    
    if (std::string("--help") == std::string(argv[i])) {
      collate_usage();
      return 0;
    }
    
    ifstream.open(argv[i]);
    
    if (ifstream.good()) {
      filenames.push_back(argv[i]);
      ifstream.close();
    } else {
      std::cout << "File " << argv[i] << " not found" << std::endl;
    }
  }
  
  if (!filenames.size()) {
    std::cout << "No valid input files, exiting." << std::endl;
    return 0;
  }
  
  std::fstream output_stream(collate_filename.c_str(),
                             std::fstream::out);
  stan::callbacks::stream_writer collate_writer(output_stream, "# ");

  // global info - must match across all sample files
  std::string model;
  std::vector<std::string> model_headers;

  for (std::vector<std::string>::size_type chain = 1; 
       chain < filenames.size(); chain++) {
    ifstream.open(filenames[chain].c_str());
    stan::io::stan_csv stan_csv = stan::io::stan_csv_reader::parse(ifstream, &std::cout);
    ifstream.close();
    if (chain == 1) {
      model = stan_csv.metadata.model;
      cmdstan::write_stan(collate_writer);
      cmdstan::write_model(collate_writer, model);
      for (size_t i = 0; i < stan_csv.header.size(); ++i) {
        model_headers.emplace_back(stan_csv.header[i]);
      }
      model_headers.emplace_back("chain");
      cmdstan::write_header(collate_writer, model_headers);
    } else {
      if (model.compare(stan_csv.metadata.model) != 0) {
        std::cout << "Error, file: " << filenames[chain]
                  << ", expecting sample from model " << model
                  << ", found sample from model " << stan_csv.metadata.model
                  << ", exiting." << std::endl;
        return 0;
      }
      if (model_headers.size() - 1 != stan_csv.header.size()) {
        std::cout << "Error, file: " << filenames[chain]
                  << ", wrong number of output columns, expecting " << model_headers.size()
                  << " columns, found " << stan_csv.header.size()
                  << " columns, exiting." << std::endl;
        return 0;
      }
      for (size_t i=0; i < stan_csv.header.size(); ++i) {
        if (model_headers[i].compare(stan_csv.header[i]) != 0) {
          std::cout << "Error, file: " << filenames[chain]
                    << ", column header names mismatch, column " << i
                    << ", expecting name " << model_headers[i]
                    << ", found name "<< stan_csv.header[i]
                    << ", exiting." << std::endl;
          return 0;
        }
      }
    }
    // write sample - add last column 
    // chain_ids.emplace_back(stan_csv.metadata.chain_id);
    // seeds.emplace_back(stan_csv.metadata.seed);
    // draws_per_chain.emplace_back(rows in sample);
  }

  // write comments at end - seeds, chain_ids, num_draws
  output_stream.close();
  return 0;
}



