#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <CLI11/CLI11.hpp>

#include <arrow/api.h>
#include <arrow/csv/api.h>
#include <arrow/filesystem/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>
#include <parquet/exception.h>


void write_parquet_file(const arrow::Table& table) {
  std::shared_ptr<arrow::io::FileOutputStream> outfile;
  PARQUET_ASSIGN_OR_THROW(
      outfile, arrow::io::FileOutputStream::Open("csv-to-arrow-example.parquet"));
  // The last argument to the function call is the size of the RowGroup in
  // the parquet file. Normally you would choose this to be rather large but
  // for the example, we use a small value to have multiple RowGroups.
  PARQUET_THROW_NOT_OK(
      parquet::arrow::WriteTable(table, arrow::default_memory_pool(), outfile, 1000));
}

struct CustomHandler {
    operator arrow::csv::InvalidRowHandler() {
      return [](const arrow::csv::InvalidRow& row) {
        return arrow::csv::InvalidRowResult::Skip;
      };
    }
};

/**
 * read in stan_csv file
 * report discovered schema
 * convert to apache parquet format
 */
int main(int argc, const char *argv[]) {
  std::string usage = R"(Usage: arrow_poc input_csv output_pqt)";
  if (argc < 2) {
    std::cout << usage << std::endl;
    return -1;
  }

  // Command-line arguments
  std::string csv_file;
  std::string pqt_file = "output.pqt";

  CLI::App app{"Allowed options"};
  app.add_option("--output_pqt,-o", pqt_file,
                 "parquet file", true)
      ->check(CLI::NonexistentPath);  // don't clobber existing file
  app.add_option("input_csv", csv_file, "Stan CSV file.", true)
      ->required()
      ->check(CLI::ExistingFile);

  try {
    CLI11_PARSE(app, argc, argv);
  } catch (const CLI::ParseError &e) {
    std::cout << e.get_exit_code();
    return app.exit(e);
  }

  // Check options semantic consistency
  if (app.count("--output_pqt")) {
    if (FILE *file = fopen(pqt_file.c_str(), "w")) {
      fclose(file);
    } else {
      std::cout << "Cannot save as parquet to file: " << pqt_file << "."
                << std::endl;
      return -1;
    }
  }
  std::ifstream infile;
  infile.open(csv_file.c_str());
  if (infile.good()) {
    infile.close();
  } else {
    std::cout << "Cannot read input csv file: " << csv_file << "."
              << std::endl;
    return -1;
  }

  try {
    // copied from https://arrow.apache.org/docs/cpp/csv.html
    arrow::io::IOContext io_context = arrow::io::default_io_context();

    auto read_options = arrow::csv::ReadOptions::Defaults();
    auto parse_options = arrow::csv::ParseOptions::Defaults();
    CustomHandler handler;
    parse_options.invalid_row_handler = handler;
    auto convert_options = arrow::csv::ConvertOptions::Defaults();

    // input csv file is on local filesystem
    std::shared_ptr<arrow::fs::LocalFileSystem> fs =
        std::make_shared<arrow::fs::LocalFileSystem>();
    auto maybe_infile = fs->OpenInputStream(csv_file.c_str());
    if (!maybe_infile.ok()) {
      // Handle instantiation error...
    }
    std::shared_ptr<arrow::io::InputStream> input = *maybe_infile;

    // Instantiate TableReader from input stream and options
    auto maybe_reader =
        arrow::csv::TableReader::Make(io_context,
                                      input,
                                      read_options,
                                      parse_options,
                                      convert_options);

    std::cout << "Instantiated reader" << std::endl;

    if (!maybe_reader.ok()) {
      // Handle TableReader instantiation error...
    }
    std::shared_ptr<arrow::csv::TableReader> reader = *maybe_reader;

    // Read table from CSV file
    auto maybe_table = reader->Read();

    std::cout << "read table" << std::endl;

    if (!maybe_table.ok()) {
      // Handle CSV read error
      // (for example a CSV syntax error or failed type conversion)
    }

    std::shared_ptr<arrow::Table> table = *maybe_table;
    std::cout << "Loaded " << table->num_rows() << " rows in " << table->num_columns()
              << " columns." << std::endl;

    // Write to csv file
    write_parquet_file(*table);    
    
  } catch (const std::exception &e) {
    std::cout << "Error during processing. " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
