#ifndef ANTARESXPANSION_SRC_CPP_HELPERS_ANTARESARCHIVEUPDATEREXEOPTIONS_H
#define ANTARESXPANSION_SRC_CPP_HELPERS_ANTARESARCHIVEUPDATEREXEOPTIONS_H
#include <filesystem>
#include <vector>

#include "OptionsParser.h"

class AntaresArchiveUpdaterExeOptions : public OptionsParser {
 private:
  std::filesystem::path archive_;
  std::vector<std::filesystem::path> paths_to_add_;
  bool delete_path_;

 public:
  AntaresArchiveUpdaterExeOptions();

  virtual ~AntaresArchiveUpdaterExeOptions() = default;
  std::filesystem::path Archive() const { return archive_; }
  std::vector<std::filesystem::path> PathsToAdd() const {
    return paths_to_add_;
  }
  bool DeletePath() const { return delete_path_; }
};
#endif  // ANTARESXPANSION_SRC_CPP_HELPERS_ANTARESARCHIVEUPDATEREXEOPTIONS_H
