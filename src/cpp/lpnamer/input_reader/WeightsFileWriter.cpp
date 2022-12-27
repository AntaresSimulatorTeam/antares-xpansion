#include "WeightsFileWriter.h"

#include <fstream>
#include <numeric>

#include "ArchiveReader.h"
#include "common_lpnamer.h"

YearlyWeightsWriter::YearlyWeightsWriter(
    const std::filesystem::path& xpansion_output_dir,
    const std::filesystem::path& zipped_output_path,
    std::vector<double> weights_vector,
    const std::filesystem::path& output_file, std::vector<int> active_years)
    : xpansion_output_dir_(xpansion_output_dir),
      zipped_output_path_(zipped_output_path),
      weights_vector_(weights_vector),
      output_file_(output_file),
      active_years_(active_years) {
  if (!std::filesystem::is_directory(xpansion_lp_dir_)) {
    xpansion_lp_dir_ = xpansion_output_dir / LP_DIR;
    std::filesystem::create_directory(xpansion_lp_dir_);
  }
}

void YearlyWeightsWriter::FillMpsWeightsMap() {
  auto zip_reader = ArchiveReader(zipped_output_path_);
  zip_reader.Open();
  zip_reader.SetEntriesPath();
  const auto& archive_files = zip_reader.EntriesPath();
  for (auto& file : archive_files) {
    if (file.extension().string() == ".mps") {
      auto year = GetYearFromMpsName(file.string());
      auto year_index = active_years_[year];
      mps_weights_[file] = weights_vector_[year_index];
    }
  }
  mps_weights_["WEIGHT_SUM"] =
      std::reduce(weights_vector_.begin(), weights_vector_.end());
}

int YearlyWeightsWriter::GetYearFromMpsName(const std::string file_name) const {
  auto split_name = common_lpnamer::split(common_lpnamer::trim(file_name), '-');
  return std::atoi(split_name[1].c_str());
}

void YearlyWeightsWriter::DumpMpsWeightsMapToFile() const {
  // auto mps_weights_file std::ofstream(xpansion_lp_dir_/output_file_.);
  for (const auto& [mps_name, weight] : mps_weights_) {
  }
}