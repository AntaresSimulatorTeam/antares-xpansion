#ifndef _ARCHIVEREADER_H
#define _ARCHIVEREADER_H
#include <functional>
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
};

class ArchiveReaderManager {
 public:
  explicit ArchiveReaderManager(const std::filesystem::path& archivePath)
      : reader_(archivePath) {
    SetMethods();
  }
  ArchiveReaderManager() = default;

 private:
  ArchiveReader reader_;
  void SetMethods();
  std::function<int32_t()> methodClose = [] { return 0; };
  int32_t Close() { return methodClose(); }

  std::function<void()> methodDelete = [] {};
  void Delete() { methodDelete(); }

  std::function<int()> methodOpen = [] { return 0; };
  int Open() { return methodOpen(); }

  std::function<int32_t(const std::filesystem::path& FileToExtractPath)>
      methodExtractFile =
          [](const std::filesystem::path& FileToExtractPath) { return 0; };
  int32_t ExtractFile(const std::filesystem::path& FileToExtractPath) {
    return methodExtractFile(FileToExtractPath);
  }

  std::function<int32_t(const std::filesystem::path& FileToExtractPath,
                        const std::filesystem::path& destination)>
      methodExtractFile2 =
          [](const std::filesystem::path& FileToExtractPath,
             const std::filesystem::path& destination) { return 0; };
  int32_t ExtractFile(const std::filesystem::path& FileToExtractPath,
                      const std::filesystem::path& destination) {
    return methodExtractFile2(FileToExtractPath, destination);
  }

  std::function<void(const std::filesystem::path& fileToExtractPath)>
      methodLocateEntry = [](const std::filesystem::path& fileToExtractPath) {};
  void LocateEntry(const std::filesystem::path& fileToExtractPath) {
    methodLocateEntry(fileToExtractPath);
  }

  std::function<void(const std::filesystem::path& fileToExtractPath)>
      methodOpenEntry = [](const std::filesystem::path& fileToExtractPath) {};
  void OpenEntry(const std::filesystem::path& fileToExtractPath) {
    methodOpenEntry(fileToExtractPath);
  }

  std::function<std::istringstream(
      const std::filesystem::path& FileToExtractPath)>
      methodExtractFileInStringStream =
          [](const std::filesystem::path& FileToExtractPath) {
            return std::istringstream(std::string(""));
          };
  std::istringstream ExtractFileInStringStream(
      const std::filesystem::path& FileToExtractPath) {
    return methodExtractFileInStringStream(FileToExtractPath);
  }

  std::function<std::filesystem::path()> methodArchivePath = []() {
    return "";
  };
  std::filesystem::path ArchivePath() const { return methodArchivePath(); }

  std::function<void(const std::filesystem::path&)> methodSetArchivePath =
      [](const std::filesystem::path& archive_path) {};
  void SetArchivePath(const std::filesystem::path& archivePath) {
    methodSetArchivePath(archivePath);
  };
};
#endif  // _ARCHIVEREADER_H
