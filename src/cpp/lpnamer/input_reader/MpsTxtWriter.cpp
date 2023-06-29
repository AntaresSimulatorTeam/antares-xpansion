
#include "MpsTxtWriter.h"

#include <fstream>

#include "ArchiveReader.h"

void FilesMapper::FillMap() {
  auto zip_reader = ArchiveReader(antares_archive_path_);
  zip_reader.Open();
  zip_reader.LoadEntriesPath();
  year_week_and_files_dict_.clear();
  FillMapWithMpsFiles(zip_reader.GetEntriesPathWithExtension(".mps"));
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
  FillMapWithVariablesFiles(variables_files);
  FillMapWithConstraintsFiles(constraints_files);
  zip_reader.Close();
  zip_reader.Delete();
}

void FilesMapper::FillMapWithMpsFiles(
    const std::vector<std::filesystem::path>& mps_files) {
  for (const auto& mps_file : mps_files) {
    year_week_and_files_dict_.try_emplace(YearAndWeekFromFileName(mps_file),
                                          mps_file, "", "");
  }
}

void FilesMapper::FillMapWithVariablesFiles(
    const std::vector<std::filesystem::path>& variables_files) {
  for (const auto& variables_file : variables_files) {
    auto year_and_week = YearAndWeekFromFileName(variables_file);
    auto& [dummy1, variables_file_field, dummy2] =
        year_week_and_files_dict_[year_and_week];
    variables_file_field = variables_file;
  }
}
void FilesMapper::FillMapWithConstraintsFiles(
    const std::vector<std::filesystem::path>& constraints_files) {
  for (const auto& constraints_file : constraints_files) {
    auto year_and_week = YearAndWeekFromFileName(constraints_file);
    auto& [dummy1, dummy2, constraints_file_field] =
        year_week_and_files_dict_[year_and_week];
    constraints_file_field = constraints_file;
  }
}

YearAndWeek FilesMapper::YearAndWeekFromFileName(
    const std::filesystem::path& file_name) const {
  auto split_file_name =
      StringManip::split(StringManip::trim(file_name.string()), '-');
  return {std::atoi(split_file_name[1].c_str()),
          std::atoi(split_file_name[2].c_str())};
}

std::vector<ProblemData> FilesMapper::MpsAndVariablesFilesVect() {
  std::vector<ProblemData> problems_data_vect;
  FillMap();
  std::transform(
      year_week_and_files_dict_.cbegin(), year_week_and_files_dict_.cend(),
      std::back_inserter(problems_data_vect),
      [](const std::pair<YearAndWeek, MpsVariableConstraintsFiles>&
             year_week_files) {
        auto& [year_week, files] = year_week_files;
        auto& [mps_file, variables_file, constraints_file] = files;
        return ProblemData(mps_file.string(), variables_file.string());
      });

  return problems_data_vect;
}
