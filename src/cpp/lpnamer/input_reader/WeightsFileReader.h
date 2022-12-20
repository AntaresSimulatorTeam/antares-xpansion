#ifndef SRC_CPP_LPNAMER_INPUTREADER_WEIGHTSFILEREADER_H
#define SRC_CPP_LPNAMER_INPUTREADER_WEIGHTSFILEREADER_H
#include <filesystem>
#include <set>

class WeightsFileReader {
 private:
  std::filesystem::path weights_file_path_;
  std::set<int> active_years_;

 public:
  explicit WeightsFileReader(const std::filesystem::path& weights_file_path,
                             const std::set<int>& active_years)
      : weights_file_path_(weights_file_path), active_years_(active_years) {}
  bool CheckWeightsFile() const;
};
#endif  // SRC_CPP_LPNAMER_INPUTREADER_WEIGHTSFILEREADER_H