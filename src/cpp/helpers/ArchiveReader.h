#ifndef _ARCHIVEREADER_H
#define _ARCHIVEREADER_H
#include <istream>
#include <sstream>
#include <string>
#include <vector>

#include "ArchiveIO.h"
class ArchiveReader : public ArchiveIO {
 private:
  void* pmz_zip_reader_instance_ = NULL;
  void* pzip_handle_ = NULL;
  std::vector<std::filesystem::path> entries_path_;
  void Create() override;
  std::filesystem::path CurrentEntryPath();
  bool entries_path_are_loaded_ = false;

 public:
  explicit ArchiveReader(const std::filesystem::path& archivePath);
  ArchiveReader();
  ~ArchiveReader() override;

  int32_t Close() override;
  void Delete() override;

  int Open() override;
  int32_t ExtractFile(const std::filesystem::path& FileToExtractPath);
  int32_t ExtractFile(const std::filesystem::path& FileToExtractPath,
                      const std::filesystem::path& destination);
  std::vector<std::filesystem::path> ExtractPattern(const std::string& pattern,
                                                    const std::string& exclude);
  std::vector<std::filesystem::path> ExtractPattern(
      const std::string& pattern, const std::string& exclude,
      const std::filesystem::path& destination);
  void LocateEntry(const std::filesystem::path& fileToExtractPath);
  void OpenEntry(const std::filesystem::path& fileToExtractPath);
  std::istringstream ExtractFileInStringStream(
      const std::filesystem::path& FileToExtractPath);
  uint64_t GetNumberOfEntries();
  std::vector<std::filesystem::path> EntriesPath() const {
    return entries_path_;
  }

  void LoadEntriesPath();
  std::vector<std::filesystem::path> GetEntriesPathWithExtension(
      const std::string& ext);
  void* InternalPointer() const { return pmz_zip_reader_instance_; }
};

#endif  // _ARCHIVEREADER_H
