#ifndef SRC_CPP_LPNAMER_INPUTREADER_MPSTXTWRITER_H
#define SRC_CPP_LPNAMER_INPUTREADER_MPSTXTWRITER_H
#include <filesystem>
#include <map>
#include <tuple>
#include <utility>
// a pair to hold double key (year and week)
using YearAndWeek = std::pair<int, int>;
// a tuple to hold files (mps, variable, constraints)
using MpsVariableConstraintsFiles =
    std::tuple<std::filesystem::path, std::filesystem::path,
               std::filesystem::path>;

// dict to associate YearAndWeek --> MpsVariableConstraintsFiles
using YearWeekAndFilesDict = std::map<YearAndWeek, MpsVariableConstraintsFiles>;
class MpsTxtWriter {
 public:
  explicit MpsTxtWriter(const std::filesystem::path& antares_archive_path)
      : antares_archive_path_(antares_archive_path) {}

 private:
  YearWeekAndFilesDict year_week_and_files_dict_;
  std::filesystem::path antares_archive_path_;
  void FillDict();
  void FillDictWithMpsFiles(
      const std::vector<std::filesystem::path>& mps_files);
  void FillDictWithVariablesFiles(
      const std::vector<std::filesystem::path>& variables_files);
  void FillDictWithConstraintsFiles(
      const std::vector<std::filesystem::path>& variables_files);
  YearAndWeek YearAndWeekFromFileName(
      const std::filesystem::path& file_name) const {
    auto split_file_name =
        common_lpnamer::split(common_lpnamer::trim(file_name.string()), '-');
    return {std::atoi(split_file_name[1].c_str()),
            std::atoi(split_file_name[2].c_str())};
  }
};
#endif  // SRC_CPP_LPNAMER_INPUTREADER_MPSTXTWRITER_H