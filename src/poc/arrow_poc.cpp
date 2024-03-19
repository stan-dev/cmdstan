#include <CLI11/CLI11.hpp>

#include <arrow/api.h>
#include <arrow/csv/api.h>
#include <arrow/io/api.h>
#include <arrow/result.h>
#include <arrow/status.h>
#include <arrow/table.h>

#include <parquet/arrow/writer.h>
#include <parquet/exception.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

struct CustomHandler {
    operator arrow::csv::InvalidRowHandler() {
      return [](const arrow::csv::InvalidRow& row) {
        return arrow::csv::InvalidRowResult::Skip;
      };
    }
};

void
check_args(const CLI::App& app, std::string& csv_file, std::string& pqt_file) {
  std::stringstream msg;
  if (app.count("--output_pqt")) {
    if (FILE *file = fopen(pqt_file.c_str(), "w")) {
      fclose(file);
    } else {
      msg << "Bad output file: " << pqt_file << std::endl;
      throw std::invalid_argument(msg.str());
    }
  }
  std::ifstream ifs;
  ifs.open(csv_file.c_str());
  if (ifs.good()) {
    ifs.close();
  } else {
    msg << "Cannot read input csv file: " << csv_file << std::endl;
    throw std::invalid_argument(msg.str());
  }
}

arrow::Result<std::shared_ptr<arrow::Table>>
read_csv_file(const std::string& csv_file) {
  std::shared_ptr<arrow::io::ReadableFile> infile;
  ARROW_ASSIGN_OR_RAISE(infile, arrow::io::ReadableFile::Open(csv_file));
  // handle Stan CSV file  
  auto parse_stan_csv_options = arrow::csv::ParseOptions::Defaults();
  CustomHandler handler;
  parse_stan_csv_options.invalid_row_handler = handler;

  ARROW_ASSIGN_OR_RAISE(
      auto csv_reader,
      arrow::csv::TableReader::Make(
          arrow::io::default_io_context(), infile,
          arrow::csv::ReadOptions::Defaults(),
          parse_stan_csv_options,
          arrow::csv::ConvertOptions::Defaults()));
  ARROW_ASSIGN_OR_RAISE(auto csv_table, csv_reader->Read());
  std::cout << "Loaded " << csv_table->num_rows()
            << " rows in " << csv_table->num_columns()
            << " columns." << std::endl;
  return csv_table;
}

void
write_parquet_file(const arrow::Result<std::shared_ptr<arrow::Table>>& ok_result,
                   const std::string &filename) {
  auto table = ok_result.ValueOrDie();
  std::shared_ptr<arrow::io::FileOutputStream> outfile;
  PARQUET_ASSIGN_OR_THROW(
      outfile, arrow::io::FileOutputStream::Open(filename));
  PARQUET_THROW_NOT_OK(
      parquet::arrow::WriteTable(*table, arrow::default_memory_pool(), outfile, 1000));
}

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
  std::string csv_file;
  std::string pqt_file = "output.parquet";
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
  check_args(app, csv_file, pqt_file);

  auto result = read_csv_file(csv_file);
  if (!result.ok()) {
    std::stringstream msg;
    msg << "Error reading CSV table: " << result.status() << std::endl;
    throw std::domain_error(msg.str());
  }
  write_parquet_file(result, pqt_file);    
  return 0;
}
