#ifndef _ARCHIVEIO_H
#define _ARCHIVEIO_H

#include <filesystem>
class ArchiveIO {
 private:
  std::filesystem::path archivePath_;

 protected:
  virtual void Create() = 0;

 public:
  explicit ArchiveIO(const std::filesystem::path& archivePath)
      : archivePath_(archivePath) {}
  ArchiveIO() = default;
  std::filesystem::path ArchivePath() const { return archivePath_; }
  void SetArchivePath(const std::filesystem::path& archivePath) {
    archivePath_ = archivePath;
  };

  virtual int32_t Close() = 0;
  virtual void Delete() = 0;

  virtual int Open() = 0;
};

#endif  // _ARCHIVEIO_H