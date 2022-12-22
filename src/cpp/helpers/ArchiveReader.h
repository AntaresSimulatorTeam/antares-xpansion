#ifndef _ARCHIVEREADER_H
#define _ARCHIVEREADER_H
#include <istream>
#include <sstream>

#include "ArchiveIO.h"
class ArchiveReader : public ArchiveIO {
 private:
  void* internalPointer_ = NULL;
  void* handle_ = NULL;
  void Create() override;

 public:
  explicit ArchiveReader(const std::filesystem::path& archivePath);
  ArchiveReader();

  int32_t Close() override;
  void Delete() override;

  int Open() override;
  int32_t ExtractFile(const std::filesystem::path& FileToExtractPath);
  int32_t ExtractFile(const std::filesystem::path& FileToExtractPath,
                      const std::filesystem::path& destination);
  void LocateEntry(const std::filesystem::path& fileToExtractPath);
  void OpenEntry(const std::filesystem::path& fileToExtractPath);
  std::istringstream ExtractFileInStringStream(
      const std::filesystem::path& FileToExtractPath);
  uint64_t GetNumberOfEntries();
  std::string GetEntryFileName(const int64_t pos);
};

#endif  // _ARCHIVEREADER_H
