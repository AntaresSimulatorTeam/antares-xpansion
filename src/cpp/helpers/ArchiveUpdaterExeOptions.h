#ifndef ANTARESXPANSION_SRC_CPP_HELPERS_ARCHIVEUPDATEREXEOPTIONS_H
#define ANTARESXPANSION_SRC_CPP_HELPERS_ARCHIVEUPDATEREXEOPTIONS_H
#include <filesystem>

#include "OptionsParser.h"
class ArchiveUpdaterExeOptions : public OptionsParser {
 private:
  std::filesystem::path archive_;
  std::filesystem::path path_to_add_;
  bool delete_path_;

 public:
  ArchiveUpdaterExeOptions();

  virtual ~ArchiveUpdaterExeOptions() = default;
  std::filesystem::path Archive() const { return archive_; }
  std::filesystem::path PathToAdd() const { return path_to_add_; }
  bool DeletePath() const { return delete_path_; }
};
#endif  // ANTARESXPANSION_SRC_CPP_HELPERS_ARCHIVEUPDATEREXEOPTIONS_H
