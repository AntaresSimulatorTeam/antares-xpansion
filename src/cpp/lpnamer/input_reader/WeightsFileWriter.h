#ifndef SRC_CPP_LPNAMER_INPUTREADER_YEARLYWEIGHTSWRITER_H
#define SRC_CPP_LPNAMER_INPUTREADER_YEARLYWEIGHTSWRITER_H
#include <filesystem>
#include <map>
#include <vector>

#include "SimulationInputMode.h"

class YearlyWeightsWriter {
 public:
  explicit YearlyWeightsWriter(const std::filesystem::path& xpansion_output_dir,
                               const std::filesystem::path& zipped_output_path,
                               const std::vector<double>& weights_vector,
                               const std::filesystem::path& output_file,
                               const std::vector<int>& active_years,
                               SimulationInputMode mode);

  void CreateWeightFile();

 private:
  std::filesystem::path xpansion_output_dir_;
  std::filesystem::path xpansion_lp_dir_ = "";
  std::filesystem::path antares_archive_path_;
  const std::string LP_DIR = "lp";
  std::map<std::filesystem::path, double> mps_weights_ = {};
  std::vector<double> weights_vector_;
  std::filesystem::path output_file_;
  std::vector<int> active_years_;
  void FillMpsWeightsMap();
  int GetYearFromMpsName(const std::string& file_name) const;
  void DumpMpsWeightsToFile() const;
  SimulationInputMode mode_;
  void FillMpsWeightsMapArchive();
  void FillMpsWeightsMapFile();
};
#endif  // SRC_CPP_LPNAMER_INPUTREADER_YEARLYWEIGHTSWRITER_H