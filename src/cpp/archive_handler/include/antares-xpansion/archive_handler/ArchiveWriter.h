#pragma once

#include <vector>

#include "ArchiveIO.h"

class ArchiveWriter : public ArchiveIO {
 private:
  void* pmz_zip_writer_instance_ = nullptr;
  void* pzip_handle_ = nullptr;
  void Create() override;
  mz_zip_file fileInfo_ = {};

  int32_t CloseInternal();
  void DeleteInternal();

 public:
  explicit ArchiveWriter(const std::filesystem::path& archivePath);
  ArchiveWriter();
  ~ArchiveWriter() override;

  int32_t Close() override;
  void Delete() override;
  int32_t CloseUnsafe();
  void DeleteUnsafe();

  int Open() override;
  void InitFileInfo();
  int32_t AddFileInArchive(const std::filesystem::path& FileToAdd);
  int32_t AddPathInArchive(const std::filesystem::path& path_to_add,
                           const std::filesystem::path& root_path);
  void* InternalPointer() const override { return pmz_zip_writer_instance_; }
};