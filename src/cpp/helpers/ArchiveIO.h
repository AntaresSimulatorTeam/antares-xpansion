#ifndef _ARCHIVEIO_H
#define _ARCHIVEIO_H

extern "C" {
#include <mz.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>
}
#include <stdexcept>
#include <string>

class ArchiveIOGeneralException : public std::runtime_error {
 public:
  ArchiveIOGeneralException(int32_t status, const std::string& action,
                            int32_t expectedStatus = MZ_OK)
      : std::runtime_error("Failed to " + action +
                           " invalid status: " + std::to_string(status) + " (" +
                           std::to_string(expectedStatus) + " expected)") {}
};
class ArchiveIOSpecificException : public std::runtime_error {
 public:
  ArchiveIOSpecificException(int32_t status, const std::string& errMessage,
                             int32_t expectedStatus = MZ_OK)
      : std::runtime_error(
            errMessage + "\ninvalid status: " + std::to_string(status) + " (" +
            std::to_string(expectedStatus) + " expected)") {}
};
#include <filesystem>
#include <shared_mutex>
class ArchiveIO {
 private:
  std::filesystem::path archivePath_;

 protected:
  mutable std::shared_mutex mutex_;

  virtual void Create() = 0;

 public:
  explicit ArchiveIO(const std::filesystem::path& archivePath)
      : archivePath_(archivePath) {}
  ArchiveIO() = default;
  virtual ~ArchiveIO() = default;
  std::filesystem::path ArchivePath() const { return archivePath_; }
  void SetArchivePath(const std::filesystem::path& archivePath) {
    archivePath_ = archivePath;
  };

  virtual int32_t Close() = 0;
  virtual void Delete() = 0;

  virtual int Open() = 0;
};

#endif  // _ARCHIVEIO_H