#ifndef _ARCHIVEWRITER_H
#define _ARCHIVEWRITER_H

#include <vector>

#include "ArchiveIO.h"
#include "FileInBuffer.h"

class ArchiveWriter : public ArchiveIO {
 private:
  void* pmz_zip_writer_instance_ = NULL;
  void* pzip_handle_ = NULL;
  void Create() override;
  mz_zip_file fileInfo_ = {0};

 public:
  explicit ArchiveWriter(const std::filesystem::path& archivePath);
  ArchiveWriter();

  int32_t Close() override;
  void Delete() override;

  int Open() override;
  void InitFileInfo();
  int32_t AddFileInArchive(const FileBuffer& FileBufferToAdd);
  int32_t AddFileInArchive(const std::filesystem::path& FileToAdd);
};
#endif  // _ARCHIVEWRITER_H