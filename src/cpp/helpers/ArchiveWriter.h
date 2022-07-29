#ifndef _ARCHIVEWRITER_H
#define _ARCHIVEWRITER_H
#include <vector>

#include "ArchiveIO.h"
#include "FileInBuffer.h"

class ArchiveWriter : public ArchiveIO {
 private:
  void* internalPointer_ = NULL;
  void* handle_ = NULL;

 public:
  explicit ArchiveWriter(const std::filesystem::path& archivePath);

  int32_t Close() override;
  void Delete() override;
  //   void* InternalPointer() const override;
  //   void* handle() const override;

  int Open() override;
  //   int32_t AddFileInArchive(const std::filesystem::path& FileToAddPath);
  int32_t AddFilesInArchive(const FileBufferVector& FilesBufferToAdd);
};
#endif  // _ARCHIVEWRITER_H