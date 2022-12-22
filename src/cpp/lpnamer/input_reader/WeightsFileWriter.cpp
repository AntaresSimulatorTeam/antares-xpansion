#include "WeightsFileWriter.h"

#include "ArchiveReader.h"

YearlyWeightsWriter::YearlyWeightsWriter(
    const std::filesystem::path& xpansion_output_dir,
    const std::filesystem::path& zipped_output_path,
    std::vector<double> weights_vector,
    const std::filesystem::path& output_file, std::set<int> active_years)
    : xpansion_output_dir_(xpansion_output_dir),
      zipped_output_path_(zipped_output_path),
      weights_vector_(weights_vector),
      output_file_(output_file),
      active_years_(active_years) {
  if (!std::filesystem::is_directory(xpansion_output_lp_dir_)) {
    xpansion_output_lp_dir_ = xpansion_output_dir / LP_DIR;
    std::filesystem::create_directory(xpansion_output_lp_dir_);
  }
}

void YearlyWeightsWriter::FillMpsWeightsMap() {
  auto zip_reader = ArchiveReader(zipped_output_path_);
  zip_reader.Open();
}