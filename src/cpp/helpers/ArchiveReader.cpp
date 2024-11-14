#include "antares-xpansion/helpers/ArchiveReader.h"

#include <iostream>
#include <regex>
#include <vector>
#include <mutex>

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

ArchiveReader::ArchiveReader(const std::filesystem::path& archivePath)
    : ArchiveIO(archivePath) {
  Create();
}
ArchiveReader::ArchiveReader() : ArchiveIO() { Create(); }
void ArchiveReader::Create() {
  std::unique_lock lock(mutex_);
  pmz_zip_reader_instance_ = mz_zip_reader_create();
}

int32_t ArchiveReader::Open() {
  if (!std::filesystem::exists(ArchivePath())) {
    std::ostringstream errMsg;
    errMsg << "Archive: " << ArchivePath().string() << " does not exist"
           << std::endl;
    throw ArchiveIOGeneralException(MZ_EXIST_ERROR, errMsg.str(), LOGLOCATION);
  }
  auto err = mz_zip_reader_open_file(pmz_zip_reader_instance_,
                                     ArchivePath().string().c_str());
  if (err != MZ_OK) {
    Close();
    Delete();
    std::ostringstream errMsg;
    errMsg << "Open Archive: " << ArchivePath().string() << std::endl;
    throw ArchiveIOGeneralException(err, errMsg.str(), LOGLOCATION);
  }
  err = mz_zip_reader_get_zip_handle(pmz_zip_reader_instance_, &pzip_handle_);
  if (err != MZ_OK) {
    Close();
    Delete();
    std::ostringstream errMsg;
    errMsg << "get underlying zip handle: " << ArchivePath().string()
           << std::endl;
    throw ArchiveIOGeneralException(err, errMsg.str(), LOGLOCATION);
  }
  return err;
}
int32_t ArchiveReader::Close() {
  std::unique_lock lock(mutex_);
  return mz_zip_reader_close(pmz_zip_reader_instance_);
}
void ArchiveReader::Delete() {
  std::unique_lock lock(mutex_);
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
           << " is not found in archive :" << ArchivePath().string().c_str()
           << std::endl;
    throw ArchiveIOSpecificException(err, errMsg.str(), LOGLOCATION);
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
    throw ArchiveIOGeneralException(err, errMsg.str(), LOGLOCATION);
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
    throw ArchiveIOGeneralException(err, errMsg.str(), LOGLOCATION);
  }
  mz_zip_reader_entry_close(pmz_zip_reader_instance_);
  return std::istringstream(std::string(buf.begin(), buf.end()));
}

uint64_t ArchiveReader::GetNumberOfEntries() {
  uint64_t number_entry = 0;
  if (auto err = mz_zip_get_number_entry(pzip_handle_, &number_entry);
      err != MZ_OK) {
    Close();
    Delete();
    std::ostringstream msg;
    msg << "get the number of entries" << std::endl;
    throw ArchiveIOGeneralException(err, msg.str(), LOGLOCATION);
  }
  return number_entry;
}

void ArchiveReader::LoadEntriesPath() {
  if (!entries_path_are_loaded_) {
    auto err = mz_zip_goto_first_entry(pzip_handle_);
    auto entry_number = 0;

    if (err != MZ_OK) {
      Close();
      Delete();
      std::ostringstream msg;
      msg << "to get first entry of archive" << ArchivePath().string().c_str()
          << std::endl;
      throw ArchiveIOGeneralException(err, msg.str(), LOGLOCATION);
    }
    entries_path_.push_back(CurrentEntryPath());
    entry_number++;

    auto number_of_entries = GetNumberOfEntries();
    while (entry_number < number_of_entries) {
      err = mz_zip_goto_next_entry(pzip_handle_);
      if (err != MZ_OK) {
        Close();
        Delete();
        std::ostringstream msg;
        msg << "get entry n° " << entry_number << std::endl;
        throw ArchiveIOGeneralException(err, msg.str(), LOGLOCATION);
      }
      entry_number++;
      entries_path_.push_back(CurrentEntryPath());
    }
    entries_path_are_loaded_ = true;
  }
}

std::filesystem::path ArchiveReader::CurrentEntryPath() {
  mz_zip_file* file_info = NULL;
  if (auto err = mz_zip_entry_get_info(pzip_handle_, &file_info);
      err != MZ_OK) {
    Close();
    Delete();
    std::ostringstream msg;
    msg << "get info from current entry " << std::endl;
    throw ArchiveIOGeneralException(err, msg.str(), LOGLOCATION);
  }
  return file_info->filename;
}

std::vector<std::filesystem::path> ArchiveReader::GetEntriesPathWithExtension(
    const std::string& ext) {
  LoadEntriesPath();
  std::vector<std::filesystem::path> result;
  std::copy_if(entries_path_.begin(), entries_path_.end(),
               std::back_inserter(result), [&ext](auto& entry_path) -> bool {
                 return entry_path.extension().string() == ext;
               });
  return result;
}

std::vector<std::filesystem::path> ArchiveReader::ExtractPattern(
    const std::string& pattern, const std::string& exclude) {
  return ExtractPattern(pattern, exclude, ArchivePath().parent_path());
}
std::vector<std::filesystem::path> ArchiveReader::ExtractPattern(
    const std::string& pattern, const std::string& exclude,
    const std::filesystem::path& destination) {
  std::unique_lock lock(mutex_);
  if (!std::filesystem::is_directory(destination)) {
    std::ostringstream msg;
    msg << "ArchiveReader::ExtractPattern destination must be a directory "
           "given: "
        << destination.string() << std::endl;
    throw ArchiveIOSpecificException(msg.str(), LOGLOCATION);
  }
  std::vector<std::filesystem::path> result;
  mz_zip_reader_set_pattern(pmz_zip_reader_instance_, pattern.c_str(), 1);
  if (mz_zip_reader_goto_first_entry(pmz_zip_reader_instance_) == MZ_OK) {
    do {
      auto current_path = CurrentEntryPath();
      auto exclude_regex = std::regex(std::string("^((?!") + exclude + ").)*$");
      auto not_excluded =
          exclude == ""
              ? true
              : std::regex_match(current_path.string(), exclude_regex);
      if (not_excluded) {
        auto target = destination / current_path;
        result.push_back(target);
        mz_zip_reader_entry_save_file(pmz_zip_reader_instance_,
                                      target.string().c_str());
      }
    } while (mz_zip_reader_goto_next_entry(pmz_zip_reader_instance_) == MZ_OK);
  }
  return result;
}
ArchiveReader::~ArchiveReader() {
  std::unique_lock lock(mutex_);
  if (pmz_zip_reader_instance_) {
    mz_zip_reader_close(pmz_zip_reader_instance_);
    mz_zip_reader_delete(&pmz_zip_reader_instance_);
  }
}
