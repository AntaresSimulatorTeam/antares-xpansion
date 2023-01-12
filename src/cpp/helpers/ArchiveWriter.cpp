#include "ArchiveWriter.h"

#include <time.h>

#include <iostream>
#include <sstream>
ArchiveWriter::ArchiveWriter(const std::filesystem::path& archivePath)
    : ArchiveIO(archivePath) {
  Create();
  InitFileInfo();
}
ArchiveWriter::ArchiveWriter() : ArchiveIO() {
  Create();
  InitFileInfo();
}

void ArchiveWriter::Create() {
  mz_zip_writer_create(&pmz_zip_writer_instance_);
}
void ArchiveWriter::InitFileInfo() {
  fileInfo_.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
  fileInfo_.flag = MZ_ZIP_FLAG_UTF8;
}
int32_t ArchiveWriter::Open() {
  // disk-spanning is disabled, meaning that only one file is created
  const auto err = mz_zip_writer_open_file(
      pmz_zip_writer_instance_, ArchivePath().string().c_str(),
      0 /* disk-spanning disabled */, 1 /* append */);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::stringstream errMsg;
    errMsg << "Open Archive: " << ArchivePath().string() << std::endl;
    throw ArchiveIOGeneralException(err, errMsg.str());
  }
  return err;
}
int32_t ArchiveWriter::Close() {
  return mz_zip_writer_close(pmz_zip_writer_instance_);
}
void ArchiveWriter::Delete() {
  mz_zip_writer_delete(&pmz_zip_writer_instance_);
}

int32_t ArchiveWriter::AddFileInArchive(const FileBuffer& FileBufferToAdd) {
  std::unique_lock lock(mutex_);
  fileInfo_.filename = FileBufferToAdd.fname.c_str();
  fileInfo_.creation_date = std::time(0);
  int32_t err = mz_zip_writer_entry_open(pmz_zip_writer_instance_, &fileInfo_);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::stringstream errMsg;
    errMsg << "Open entry: " << FileBufferToAdd.fname
           << " in archive: " << ArchivePath().string() << std::endl;
    throw ArchiveIOGeneralException(err, errMsg.str());
  }
  const auto len = FileBufferToAdd.buffer.size();
  auto bw = mz_zip_writer_entry_write(pmz_zip_writer_instance_,
                                      FileBufferToAdd.buffer.c_str(), len);
  if (bw != len) {
    Close();
    Delete();
    std::stringstream errMsg;
    errMsg << "[KO] mz_zip_writer_entry_write, expected " << len
           << "data to be read got" << bw << std::endl;
    throw ArchiveIOSpecificException(err, errMsg.str());
  }
  err = mz_zip_writer_entry_close(pmz_zip_writer_instance_);
  if (MZ_OK != err) {
    Close();
    Delete();
    std::stringstream errMsg;
    errMsg << "[KO] mz_zip_writer_entry_close error: could not close entry: "
           << FileBufferToAdd.fname << std::endl;
    throw ArchiveIOSpecificException(err, errMsg.str());
  }

  return MZ_OK;
}
int32_t ArchiveWriter::AddFileInArchive(
    const std::filesystem::path& FileToAdd) {
  std::unique_lock lock(mutex_);
  if (auto err = mz_zip_writer_add_file(pmz_zip_writer_instance_,
                                        FileToAdd.string().c_str(),
                                        FileToAdd.filename().string().c_str());
      err != MZ_OK) {
    Close();
    Delete();
    std::stringstream errMsg;
    errMsg << "[KO] mz_zip_writer_add_file: Failed to add file: " << FileToAdd
           << " in archive: " << ArchivePath().string() << std::endl;
    throw ArchiveIOSpecificException(err, errMsg.str());
  }

  return MZ_OK;
}
int32_t ArchiveWriter::AddPathInArchive(
    const std::filesystem::path& path_to_add,
    const std::filesystem::path& root_path) {
  std::unique_lock lock(mutex_);
  if (auto err = mz_zip_writer_add_path(pmz_zip_writer_instance_,
                                        path_to_add.string().c_str(),
                                        root_path.string().c_str(), 0, 1);
      err != MZ_OK) {
    Close();
    Delete();
    std::stringstream errMsg;
    errMsg << "[KO] mz_zip_writer_add_path: Failed to add path: " << path_to_add
           << " in archive: " << ArchivePath().string() << std::endl;
    throw ArchiveIOSpecificException(err, errMsg.str());
  }

  return MZ_OK;
}
