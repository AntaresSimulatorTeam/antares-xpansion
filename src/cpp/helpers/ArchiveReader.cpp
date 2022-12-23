#include "ArchiveReader.h"

#include <iostream>
#include <vector>
ArchiveReader::ArchiveReader(const std::filesystem::path& archivePath)
    : ArchiveIO(archivePath) {
  Create();
}
ArchiveReader::ArchiveReader() : ArchiveIO() { Create(); }
void ArchiveReader::Create() {
  mz_zip_reader_create(&pmz_zip_reader_instance_);
}

int32_t ArchiveReader::Open() {
  auto err = mz_zip_reader_open_file(pmz_zip_reader_instance_,
                                     ArchivePath().string().c_str());
  if (err != MZ_OK) {
    Close();
    Delete();
    std::ostringstream errMsg;
    errMsg << "Open Archive: " << ArchivePath().string() << std::endl;
    throw ArchiveIOGeneralException(err, errMsg.str());
  }
  err = mz_zip_reader_get_zip_handle(pmz_zip_reader_instance_, &pzip_handle_);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::ostringstream errMsg;
    errMsg << "get underlying zip handle: " << ArchivePath().string()
           << std::endl;
    throw ArchiveIOGeneralException(err, errMsg.str());
  }
  return err;
}
int32_t ArchiveReader::Close() {
  return mz_zip_reader_close(pmz_zip_reader_instance_);
}
void ArchiveReader::Delete() {
  mz_zip_reader_delete(&pmz_zip_reader_instance_);
}

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
  err = mz_zip_reader_entry_save_file(pmz_zip_reader_instance_,
                                      targetFile.string().c_str());
  mz_zip_reader_entry_close(pmz_zip_reader_instance_);
  return err;
}
void ArchiveReader::LocateEntry(
    const std::filesystem::path& fileToExtractPath) {
  auto err = mz_zip_reader_locate_entry(pmz_zip_reader_instance_,
                                        fileToExtractPath.string().c_str(), 1);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::ostringstream errMsg;
    errMsg << "File : " << fileToExtractPath.string().c_str()
           << " is not found in archive :" << ArchivePath().c_str()
           << std::endl;
    throw ArchiveIOSpecificException(err, errMsg.str());
  }
}
void ArchiveReader::OpenEntry(const std::filesystem::path& fileToExtractPath) {
  auto err = mz_zip_reader_entry_open(pmz_zip_reader_instance_);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::ostringstream errMsg;
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
  int32_t len =
      mz_zip_reader_entry_save_buffer_length(pmz_zip_reader_instance_);
  std::vector<char> buf(len);
  auto err = mz_zip_reader_entry_save_buffer(pmz_zip_reader_instance_,
                                             buf.data(), len);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::ostringstream errMsg;
    errMsg << "Extract file " << FileToExtractPath.string()
           << "in archive: " << ArchivePath().string() << std::endl;
    throw ArchiveIOGeneralException(err, errMsg.str());
  }
  mz_zip_reader_entry_close(pmz_zip_reader_instance_);
  return std::istringstream(std::string(buf.begin(), buf.end()));
}

uint64_t ArchiveReader::GetNumberOfEntries() {
  uint64_t number_entry = 0;
  auto err = mz_zip_get_number_entry(pzip_handle_, &number_entry);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::ostringstream msg;
    msg << "get the number of entries" << std::endl;
    throw ArchiveIOGeneralException(err, msg.str());
  }
  return number_entry;
}

std::string ArchiveReader::GetEntryFileName(const int64_t pos) {
  auto err = mz_zip_goto_entry(pzip_handle_, pos);
  // auto err = mz_zip_goto_first_entry(pmz_zip_reader_instance_);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::ostringstream msg;
    msg << "find entry at position: " << pos << std::endl;
    throw ArchiveIOGeneralException(err, msg.str());
  }

  mz_zip_file* file_info = NULL;
  err = mz_zip_entry_get_info(pzip_handle_, &file_info);
  // err = mz_zip_entry_get_info(pmz_zip_reader_instance_, &file_info);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::ostringstream msg;
    msg << "get entry info at position: " << pos << std::endl;
    throw ArchiveIOGeneralException(err, msg.str());
  }
  return file_info->filename;
}