
#include "MpsTxtWriter.h"

#include <cstdlib>
#include <fstream>

#include "ArchiveReader.h"

void MpsTxtWriter::FillDict() {
  auto zip_reader = ArchiveReader(antares_archive_path_);
  zip_reader.Open();
  zip_reader.LoadEntriesPath();
  year_week_and_files_dict_.clear();
  FillDictWithMpsFiles(zip_reader.GetEntriesPathWithExtension(".mps"));
  std::vector<std::filesystem::path> variables_files;
  std::vector<std::filesystem::path> constraints_files;
  auto vect_of_files_with_txt_ext =
      zip_reader.GetEntriesPathWithExtension(".txt");
  for (const auto& file : vect_of_files_with_txt_ext) {
    auto filename_str = file.stem().string();
    auto is_it_variables_file =
        filename_str.find("variables") != std::string::npos;
    auto is_it_constraints_file =
        filename_str.find("constraints") != std::string::npos;
    if (is_it_variables_file && is_it_constraints_file) {
      // case file : ***variables**constraints***.txt or
      // ***constraints**variables***.txt
      continue;
    } else if (is_it_variables_file) {
      variables_files.emplace_back(file);
    } else if (is_it_constraints_file) {
      constraints_files.emplace_back(file);
    }
  }
  FillDictWithVariablesFiles(variables_files);
  FillDictWithConstraintsFiles(constraints_files);
}

void MpsTxtWriter::FillDictWithMpsFiles(
    const std::vector<std::filesystem::path>& mps_files) {
  for (const auto& mps_file : mps_files) {
    year_week_and_files_dict_.insert(
        {YearAndWeekFromFileName(mps_file), {mps_file, "", ""}});
  }
}

void MpsTxtWriter::FillDictWithVariablesFiles(
    const std::vector<std::filesystem::path>& variables_files) {
  for (const auto& variables_file : variables_files) {
    auto year_and_week = YearAndWeekFromFileName(variables_file);
    auto& [dummy1, variables_file_field, dummy2] =
        year_week_and_files_dict_[year_and_week];
    variables_file_field = variables_file;
  }
}
void MpsTxtWriter::FillDictWithConstraintsFiles(
    const std::vector<std::filesystem::path>& constraints_files) {
  for (const auto& constraints_file : constraints_files) {
    auto year_and_week = YearAndWeekFromFileName(constraints_file);
    auto& [dummy1, dummy2, constraints_file_field] =
        year_week_and_files_dict_[year_and_week];
    constraints_file_field = constraints_file;
  }
}

void MpsTxtWriter::Write() {
  FillDict();
  std::ofstream output_file(output_file_path_);
  for (const auto& [year_and_week, files] : year_week_and_files_dict_) {
    auto& [mps_file, vars_file, constr_file] = files;
    output_file << mps_file.string() << " " << vars_file.string() << " "
                << constr_file.string() << std::endl;
  }
  output_file.close();
}