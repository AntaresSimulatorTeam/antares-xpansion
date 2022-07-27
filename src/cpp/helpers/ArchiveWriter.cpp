#include "ArchiveWriter.h"

#include <mz.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

#include <iostream>
ArchiveWriter::ArchiveWriter(const std::filesystem::path& archivePath)
    : ArchiveIO(archivePath) {
  mz_zip_writer_create(&internalPointer_);
}
// void* ArchiveWriter::InternalPointer() const { return mz_zip_reader; }

int32_t ArchiveWriter::Open() {
  const auto err =
      mz_zip_writer_open_file(internalPointer_, ArchivePath().c_str(), 0, 1);
  if (err != MZ_OK) {
    std::cerr << "Could not Open Archive: " << ArchivePath().string()
              << std::endl;
  }
  return err;
}
int32_t ArchiveWriter::Close() { return mz_zip_writer_close(internalPointer_); }
void ArchiveWriter::Delete() { mz_zip_writer_delete(&internalPointer_); }

int32_t ArchiveWriter::AddFileInArchive(const FileBuffer& FileBufferToAdd) {
  mz_zip_file file_info = {0};
  file_info.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
  file_info.flag = MZ_ZIP_FLAG_UTF8;
  file_info.filename = FileBufferToAdd.fname.c_str();
  int32_t err = mz_zip_writer_entry_open(internalPointer_, &file_info);
  if (err != MZ_OK) {
    std::cerr << "Failed to add file: " << FileBufferToAdd.fname
              << " in archive" << std::endl;
    return err;
  }
  const auto len = FileBufferToAdd.buffer.size();
  auto bw = mz_zip_writer_entry_write(internalPointer_,
                                      FileBufferToAdd.buffer.c_str(), len);
  if (bw != len) {
    std::cerr << "[KO] mz_zip_writer_entry_write " << bw << std::endl;
    return -1;
  }
  err = mz_zip_writer_entry_close(internalPointer_);
  if (MZ_OK != err) {
    std::cerr << "[KO] mz_zip_writer_entry_close error: " << err << std::endl;
    return err;
  }
  return MZ_OK;
}
