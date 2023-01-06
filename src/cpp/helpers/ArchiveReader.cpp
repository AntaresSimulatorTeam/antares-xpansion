#include "ArchiveReader.h"

#include <iostream>
#include <vector>
ArchiveReader::ArchiveReader(const std::filesystem::path& archivePath)
    : ArchiveIO(archivePath) {
  Create();
}
ArchiveReader::ArchiveReader() : ArchiveIO() { Create(); }
void ArchiveReader::Create() { mz_zip_reader_create(&internalPointer_); }

int32_t ArchiveReader::Open() {
  const auto err =
      mz_zip_reader_open_file(internalPointer_, ArchivePath().string().c_str());
  if (err != MZ_OK) {
    Close();
    Delete();
    std::stringstream errMsg;
    errMsg << "Open Archive: " << ArchivePath().string() << std::endl;
    throw ArchiveIOGeneralException(err, errMsg.str());
  }
  return err;
}
int32_t ArchiveReader::Close() { return mz_zip_reader_close(internalPointer_); }
void ArchiveReader::Delete() { mz_zip_reader_delete(&internalPointer_); }

int32_t ArchiveReader::ExtractFile(
    const std::filesystem::path& fileToExtractPath) {
  return ExtractFile(fileToExtractPath, ArchivePath().parent_path() /
                                            fileToExtractPath.filename());
}

int32_t ArchiveReader::ExtractFile(
    const std::filesystem::path& fileToExtractPath,
    const std::filesystem::path& destination) {
  std::unique_lock lock(mutex_);
  int32_t err = MZ_OK;
  LocateEntry(fileToExtractPath);
  OpenEntry(fileToExtractPath);

  std::filesystem::path targetFile(destination);
  if (std::filesystem::is_directory(destination)) {
    targetFile = destination / fileToExtractPath.filename();
  }
  err = mz_zip_reader_entry_save_file(internalPointer_,
                                      targetFile.string().c_str());
  mz_zip_reader_entry_close(internalPointer_);
  return err;
}
void ArchiveReader::LocateEntry(
    const std::filesystem::path& fileToExtractPath) {
  auto err = mz_zip_reader_locate_entry(internalPointer_,
                                        fileToExtractPath.string().c_str(), 1);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::stringstream errMsg;
    errMsg << "File : " << fileToExtractPath.string().c_str()
           << " is not found in archive :" << ArchivePath().string().c_str()
           << std::endl;
    throw ArchiveIOSpecificException(err, errMsg.str());
  }
}
void ArchiveReader::OpenEntry(const std::filesystem::path& fileToExtractPath) {
  auto err = mz_zip_reader_entry_open(internalPointer_);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::stringstream errMsg;
    errMsg << "open " << fileToExtractPath.string()
           << " in archive :" << ArchivePath().string() << std::endl;
    throw ArchiveIOGeneralException(err, errMsg.str());
  }
}
std::istringstream ArchiveReader::ExtractFileInStringStream(
    const std::filesystem::path& FileToExtractPath) {
  std::unique_lock lock(mutex_);
  LocateEntry(FileToExtractPath);
  OpenEntry(FileToExtractPath);
  int32_t len = mz_zip_reader_entry_save_buffer_length(internalPointer_);
  std::vector<char> buf(len);
  auto err = mz_zip_reader_entry_save_buffer(internalPointer_, buf.data(), len);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::stringstream errMsg;
    errMsg << "Extract file " << FileToExtractPath.string()
           << "in archive: " << ArchivePath().string() << std::endl;
    throw ArchiveIOGeneralException(err, errMsg.str());
  }
  mz_zip_reader_entry_close(internalPointer_);
  return std::istringstream(std::string(buf.begin(), buf.end()));
}
