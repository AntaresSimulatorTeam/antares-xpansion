#include "ArchiveReader.h"

#include <mz.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

#include <iostream>
ArchiveReader::ArchiveReader(const std::filesystem::path& archivePath)
    : ArchiveIO(archivePath) {
  mz_zip_reader_create(&internalPointer_);
}
// void* ArchiveReader::InternalPointer() const { return mz_zip_reader; }

int32_t ArchiveReader::Open() {
  const auto err =
      mz_zip_reader_open_file(internalPointer_, ArchivePath().c_str());
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
  auto fileNameCCharPtr = fileToExtractPath.c_str();
  int32_t err = MZ_OK;
  err = mz_zip_reader_locate_entry(internalPointer_, fileNameCCharPtr, 1);
  if (err != MZ_OK) {
    std::cerr << "File : " << fileNameCCharPtr
              << " is not found in archive :" << ArchivePath().c_str()
              << std::endl;
    return err;
  }
  err = mz_zip_reader_entry_open(internalPointer_);
  if (err != MZ_OK) {
    std::cerr << "Could not open : " << fileNameCCharPtr
              << " in archive :" << ArchivePath().c_str() << std::endl;
    return err;
  }

  if (std::filesystem::is_directory(destination)) {
    auto targetFile = destination / fileToExtractPath.filename();
    auto bytes_read =
        mz_zip_reader_entry_save_file(internalPointer_, targetFile.c_str());
  } else {
    auto bytes_read =
        mz_zip_reader_entry_save_file(internalPointer_, destination.c_str());
  }
  mz_zip_reader_entry_close(internalPointer_);
  return err;
}