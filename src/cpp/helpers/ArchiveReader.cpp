#include "ArchiveReader.h"

#include <mz.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

#include <iostream>
ArchiveReader::ArchiveReader(const std::filesystem::path& archivePath)
    : ArchiveIO(archivePath) {
  Create();
}
// void* ArchiveReader::InternalPointer() const { return mz_zip_reader; }
ArchiveReader::ArchiveReader() : ArchiveIO() { Create(); }
void ArchiveReader::Create() { mz_zip_reader_create(&internalPointer_); }

int32_t ArchiveReader::Open() {
  const auto err =
      mz_zip_reader_open_file(internalPointer_, ArchivePath().string().c_str());
  if (err != MZ_OK) {
    std::cerr << "Could not Open Archive: " << ArchivePath().string()
              << std::endl;
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
  int32_t err = MZ_OK;
  // err = mz_zip_reader_locate_entry(internalPointer_,
  //                                  fileToExtractPath.string().c_str(), 1);
  // if (err != MZ_OK) {
  //   std::cerr << "File : " << fileToExtractPath.string().c_str()
  //             << " is not found in archive :" << ArchivePath().c_str()
  //             << std::endl;
  //   return err;
  // }
  err = LocateEntry(fileToExtractPath);
  err = OpenEntry(fileToExtractPath);
  // err = mz_zip_reader_entry_open(internalPointer_);
  // if (err != MZ_OK) {
  //   std::cerr << "Could not open : " << fileToExtractPath.string().c_str()
  //             << " in archive :" << ArchivePath().c_str() << std::endl;
  //   return err;
  // }

  if (std::filesystem::is_directory(destination)) {
    auto targetFile = destination / fileToExtractPath.filename();
    auto bytes_read = mz_zip_reader_entry_save_file(
        internalPointer_, targetFile.string().c_str());
  } else {
    auto bytes_read = mz_zip_reader_entry_save_file(
        internalPointer_, destination.string().c_str());
  }
  mz_zip_reader_entry_close(internalPointer_);
  return err;
}
int32_t ArchiveReader::LocateEntry(
    const std::filesystem::path& fileToExtractPath) {
  auto err = mz_zip_reader_locate_entry(internalPointer_,
                                        fileToExtractPath.string().c_str(), 1);
  if (err != MZ_OK) {
    std::cerr << "File : " << fileToExtractPath.string().c_str()
              << " is not found in archive :" << ArchivePath().c_str()
              << std::endl;
  }
  return err;
}
int32_t ArchiveReader::OpenEntry(
    const std::filesystem::path& fileToExtractPath) {
  auto err = mz_zip_reader_entry_open(internalPointer_);
  if (err != MZ_OK) {
    std::cerr << "Could not open : " << fileToExtractPath.string().c_str()
              << " in archive :" << ArchivePath().c_str() << std::endl;
    return err;
  }
}
std::istringstream ArchiveReader::ExtractFileInStringStream(
    const std::filesystem::path& FileToExtractPath) {
  auto err = LocateEntry(FileToExtractPath);
  err = OpenEntry(FileToExtractPath);
  int32_t len = mz_zip_reader_entry_save_buffer_length(internalPointer_);
  std::vector<char> buf(len);
  err = mz_zip_reader_entry_save_buffer(internalPointer_, buf.data(), len);
  // if(err!= MZ_OK){
  //   std::cerr<< "Error "
  // }
  mz_zip_reader_entry_close(internalPointer_);
  return std::istringstream(std::string(buf.begin(), buf.end()));
}