#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include <utility>
#include <vector>
#include <CLI11/CLI11.hpp>

#define PROFILE_CSV_LINE_LENGTH 9


struct profile_data{
  double time_total;
  double forward_time;
  double reverse_time;
  size_t chain_stack_total;
  size_t nochain_stack_total;
  size_t no_autodiff_passes;
  size_t autodiff_passes;
  size_t n_threads;
};

using profile_data_map = std::map<std::string, profile_data>;
profile_data_map profile_map; 

/**
 * Compute the summary of profiling data 
 * from CSV files with Stan profiling data.
 * Command line options handled by CLI11 lib.
 *
 * @param argc Number of arguments
 * @param argv Arguments
 *
 * @return 0 for success,
 *         non-zero otherwise
 */
int main(int argc, const char *argv[]) {
  std::string usage = R"(Usage: profilesummary [OPTIONS] stan_profiling_csv_file(s)
Report summaries of the profiling data CSV files with Stan profiling data.
Example:  profilesummary profiling_chain_1.csv profiling_chain_2.csv
Options:
  -h, --help                  Produce help message, then exit.
  -s, --sig_figs [n]          Significant figures reported. Default is 2.
                              Must be an integer from (1, 18), inclusive.
)";
  if (argc < 2) {
    std::cout << usage << std::endl;
    return -1;
  }

  // Command-line arguments
  int sig_figs = 4;
  std::string csv_filename;
  std::vector<std::string> filenames;

  CLI::App app{"Allowed options"};
  app.add_option("--sig_figs,-s", sig_figs, "Significant figures, default 4.",
                 true)
      ->check(CLI::Range(1, 18));
  app.add_option("input_files", filenames, "Stan profiling CSV files.", true)
      ->required()
      ->check(CLI::ExistingFile);

  try {
    CLI11_PARSE(app, argc, argv);
  } catch (const CLI::ParseError &e) {
    std::cout << e.get_exit_code();
    return app.exit(e);
  }

  for (int i = 0; i < filenames.size(); ++i) {
    std::ifstream infile;
    infile.open(filenames[i].c_str());
    if (infile.good()) {
      infile.close();
    } else {
      std::cout << "Cannot read input csv file: " << filenames[i] << "."
                << std::endl;
      return -1;
    }
  }
  std::string delimiter = ",";
  
  try {
    for (int i = 0; i < filenames.size(); ++i) {
      std::string line;
      std::ifstream infile;
      infile.open(filenames[i].c_str());
 
      std::getline(infile, line); // ignore header
      while(std::getline(infile, line, '\n')) 
      { 
        std::stringstream line_stream(line);
        std::vector<std::string> line_cells;
        std::string cell;
        while(std::getline(line_stream,cell, ','))
        {
            line_cells.push_back(cell);
        }
        if(line_cells.size() != PROFILE_CSV_LINE_LENGTH) {
          std::cerr << "Expected " << PROFILE_CSV_LINE_LENGTH << " but found " 
          << line_cells.size() << " columns!" << "!" << std::endl;
          return -1;
        }
        std::string name = line_cells[0];
        profile_data_map::iterator it = profile_map.find(name);
        if (it == profile_map.end()) {
          profile_map[name] = {};
        }
        profile_data& p = profile_map[name];
        p.time_total += std::stod(line_cells[2]);
        p.forward_time += std::stod(line_cells[3]);
        p.reverse_time += std::stod(line_cells[4]);
        p.chain_stack_total += std::stoi(line_cells[5]);
        p.nochain_stack_total += std::stoi(line_cells[6]);
        p.no_autodiff_passes += std::stoi(line_cells[7]);
        p.autodiff_passes += std::stoi(line_cells[8]);
        p.n_threads += 1;
      }
      infile.close();
    }

    size_t n_files = filenames.size();


    profile_data_map::iterator it;
    for (it = profile_map.begin(); it != profile_map.end(); it++)
    {
      if (it->second.n_threads > n_files) {
          double ratio = n_files/it->second.n_threads;
          it->second.time_total *= ratio;
          it->second.forward_time *= ratio;
          it->second.reverse_time *= ratio;
          it->second.no_autodiff_passes *= ratio;
          it->second.autodiff_passes *= ratio;
      }
    }
    
    std::cout << std::setprecision(sig_figs)
    << "Profile information combined from " << n_files << " file(s)"
    << std::endl << std::endl
    << "Timing information: " << std::endl << std::endl
    // TODO: These tabs are obviously useless
    << "\t\t" << "Total" << "\t\t" << "Forward pass" << "\t\t" << "Reverse pass" <<  std::endl;
    profile_data_map::iterator it;
    for (it = profile_map.begin(); it != profile_map.end(); it++)
    {
        std::cout << it->first 
        << "\t\t" << it->second.time_total
        << "\t\t"<< it->second.forward_time
        << "\t\t"<< it->second.reverse_time << std::endl;
    }
    std::cout << std::endl << "Memory information: " << std::endl << std::endl
    << "\t\t" << "ChainStack" << "\t\t" << "NoChainStack" <<  std::endl;
    for (it = profile_map.begin(); it != profile_map.end(); it++)
    {
        std::cout << it->first 
        << "\t\t" << it->second.chain_stack_total
        << "\t\t"<< it->second.nochain_stack_total
        << std::endl;
    }
    std::cout << std::endl << "Autodiff information: " << std::endl << std::endl
    << "\t\t" << "NoADPasses" << "\t\t" << "AdPasses" <<  std::endl;
    for (it = profile_map.begin(); it != profile_map.end(); it++)
    {
        std::cout << it->first 
        << "\t\t" << it->second.no_autodiff_passes
        << "\t\t"<< it->second.autodiff_passes
        << std::endl;
    }    
  } catch (const std::invalid_argument &e) {
    std::cerr << "Error during processing. " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
