#include "WeightsFileWriter.h"

#include <fstream>
#include <numeric>

#include "ArchiveReader.h"
#include "StringManip.h"

YearlyWeightsWriter::YearlyWeightsWriter(
    const std::filesystem::path& xpansion_output_dir,
    const std::filesystem::path& antares_archive_path,
    const std::vector<double>& weights_vector,
    const std::filesystem::path& output_file,
    const std::vector<int>& active_years)
    : xpansion_output_dir_(xpansion_output_dir),
      antares_archive_path_(antares_archive_path),
      weights_vector_(weights_vector),
      output_file_(output_file),
      active_years_(active_years) {
  xpansion_lp_dir_ = xpansion_output_dir / LP_DIR;
  if (!std::filesystem::is_directory(xpansion_lp_dir_)) {
    std::filesystem::create_directory(xpansion_lp_dir_);
  }
}

void YearlyWeightsWriter::CreateWeightFile() {
  FillMpsWeightsMap();
  DumpMpsWeightsToFile();
}

void YearlyWeightsWriter::FillMpsWeightsMap() {
  mps_weights_.clear();
  auto zip_reader = ArchiveReader(antares_archive_path_);
  zip_reader.Open();
  zip_reader.LoadEntriesPath();
  const auto& mps_files = zip_reader.GetEntriesPathWithExtension(".mps");
  for (auto& mps_file : mps_files) {
    auto year = GetYearFromMpsName(mps_file.string());
    auto year_index =
        std::find(active_years_.begin(), active_years_.end(), year) -
        active_years_.begin();
    mps_weights_[mps_file.filename()] = weights_vector_[year_index];
  }
  zip_reader.Close();
  zip_reader.Delete();
}

int YearlyWeightsWriter::GetYearFromMpsName(
    const std::string& file_name) const {
  auto split_name = StringManip::split(StringManip::trim(file_name), '-');
  return std::stoi(split_name[1]);
}

void YearlyWeightsWriter::DumpMpsWeightsToFile() const {
  auto file = xpansion_lp_dir_ / output_file_.filename().string();
  std::ofstream mps_weights_file;
  mps_weights_file.open(file);

  for (const auto& [mps_name, weight] : mps_weights_) {
    mps_weights_file << mps_name.string() << " " << weight << std::endl;
  }
  mps_weights_file << "WEIGHT_SUM "
                   << std::reduce(weights_vector_.begin(),
                                  weights_vector_.end())
                   << std::endl;
  mps_weights_file.close();
}