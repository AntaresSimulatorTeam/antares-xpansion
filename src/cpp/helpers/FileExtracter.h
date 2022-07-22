#ifndef __FILEEXTRACTER_H__
#define __FILEEXTRACTER_H__
#include <filesystem>
#include <string>

#include "libzippp.h"
class FileExtracter {
 private:
  std::filesystem::path archiveName_;
  std::string fileName_;
  libzippp::ZipArchive archive_;
  bool OpenArchive_() { return archive_.open(libzippp::ZipArchive::ReadOnly); }

 public:
  explicit FileExtracter(const std::filesystem::path& archiveName,
                         const std::string& fileName);
  std::filesystem::path GetExctractedFilePath();
};

#endif  // __FILEEXTRACTER_H__
