#ifndef SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
#define SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
#include <filesystem>

class LpFilesExtractor {
 private:
  std::filesystem::path antares_archive_path_;
  std::filesystem::path xpansion_lp_dir_;

 public:
  explicit LpFilesExtractor(const std::filesystem::path& antares_archive_path,
                            const std::filesystem::path& xpansion_lp_dir)
      : antares_archive_path_(antares_archive_path),
        xpansion_lp_dir_(xpansion_lp_dir) {}
  void ExtractFiles() const;
};
#endif  // SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
