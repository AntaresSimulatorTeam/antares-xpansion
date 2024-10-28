#ifndef SRC_CPP_LPNAMER_INPUTREADER_MPSTXTWRITER_H
#define SRC_CPP_LPNAMER_INPUTREADER_MPSTXTWRITER_H
#include <cstdlib>
#include <filesystem>
#include <map>
#include <tuple>
#include <utility>
#include <vector>

#include "antares-xpansion/xpansion_interfaces/StringManip.h"

// a pair to hold double key (year and week)
using YearAndWeek = std::pair<int, int>;
// a tuple to hold files (mps, variable, constraints)
using MpsVariableConstraintsFiles =
    std::tuple<std::filesystem::path, std::filesystem::path,
               std::filesystem::path>;

// dict to associate YearAndWeek --> MpsVariableConstraintsFiles
using YearWeekAndFilesDict = std::map<YearAndWeek, MpsVariableConstraintsFiles>;

struct ProblemData {
  ProblemData(const std::string& problem_mps, const std::string& variables_txt)
      : _problem_mps(problem_mps), _variables_txt(variables_txt) {}
  std::string _problem_mps;
  std::string _variables_txt;
};

class FilesMapper {
 public:
  explicit FilesMapper(std::filesystem::path antares_archive_path,
                       std::filesystem::path simulation_dir = {})
      : antares_archive_path_(std::move(antares_archive_path)),
        simulation_dir_(std::move(simulation_dir)) {}
  YearWeekAndFilesDict FilesMap() {
    FillMap();
    return year_week_and_files_dict_;
  }
  std::vector<ProblemData> MpsAndVariablesFilesVect();

 private:
  YearWeekAndFilesDict year_week_and_files_dict_;
  std::filesystem::path antares_archive_path_;
  std::filesystem::path simulation_dir_;
  void FillMap();
  void FillMapWithMpsFiles(const std::vector<std::filesystem::path>& mps_files);
  void FillMapWithVariablesFiles(
      const std::vector<std::filesystem::path>& variables_files);
  void FillMapWithConstraintsFiles(
      const std::vector<std::filesystem::path>& variables_files);
  YearAndWeek YearAndWeekFromFileName(
      const std::filesystem::path& file_name) const;
  void FillMapFiles();
  void FillMapZip();
};
#endif  // SRC_CPP_LPNAMER_INPUTREADER_MPSTXTWRITER_H