#ifndef _ARCHIVEIO_H
#define _ARCHIVEIO_H

#include <filesystem>
class ArchiveIO {
 private:
  std::filesystem::path archivePath_;

 public:
  std::filesystem::path ArchivePath() const { return archivePath_; }

 public:
  explicit ArchiveIO(const std::filesystem::path& archivePath)
      : archivePath_(archivePath) {}
  // virtual void* InternalPointer() const = 0;
  // virtual void* handle() const = 0;
  virtual int32_t Close() = 0;
  virtual void Delete() = 0;

  virtual int Open() = 0;
};

#endif  // _ARCHIVEIO_H