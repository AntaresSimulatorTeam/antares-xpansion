#include "antares-xpansion/helpers/AntaresArchiveUpdater.h"

#include <iostream>
#include <vector>

#include "antares-xpansion/helpers/ArchiveReader.h"
#include "antares-xpansion/xpansion_interfaces/StringManip.h"

const std::string CRITERION_FILES_PREFIX = "criterion";
const std::string CONSTRAINTS_FILES_PREFIX = "constraints";
const std::string VARIABLES_FILES_PREFIX = "variables";
const std::string INTERCO_FILES_PREFIX = "interco";
const std::string AREA_FILES_PREFIX = "area";
const std::string MPS_FILES_EXTENSION = ".mps";

bool isCriterionFile(const std::filesystem::path &file_name) {
  auto file_name_str = file_name.string();
  return StringManip::StringUtils::contains(file_name_str,
                                            CRITERION_FILES_PREFIX) &&
         !file_name.has_root_path();
}
bool isVariablesFile(const std::filesystem::path &file_name) {
  auto file_name_str = file_name.string();
  return StringManip::StringUtils::contains(file_name_str,
                                            VARIABLES_FILES_PREFIX) &&
         !file_name.has_root_path();
}
bool isConstraintsFile(const std::filesystem::path &file_name) {
  auto file_name_str = file_name.string();
  return StringManip::StringUtils::contains(file_name_str,
                                            CONSTRAINTS_FILES_PREFIX) &&
         !file_name.has_root_path();
}

bool isAreaFile(const std::filesystem::path &file_name) {
  auto file_name_str = file_name.string();
  return StringManip::StringUtils::contains(file_name_str, AREA_FILES_PREFIX) &&
         !file_name.has_root_path();
}

bool isIntercoFile(const std::filesystem::path &file_name) {
  auto file_name_str = file_name.string();
  return StringManip::StringUtils::contains(file_name_str,
                                            INTERCO_FILES_PREFIX) &&
         !file_name.has_root_path();
}

bool IsAntaresMpsFile(const std::filesystem::path &file_name) {
  return file_name.filename().extension() == MPS_FILES_EXTENSION &&
         !file_name.has_parent_path();
}

int32_t AntaresArchiveUpdater::MinizipErase(void *reader, void *writer) {
  mz_zip_file *file_info = NULL;

  int32_t err = MZ_OK;
  uint8_t zip_cd = 0;

  err = mz_zip_reader_goto_first_entry(reader);

  if (err != MZ_OK && err != MZ_END_OF_LIST)
    std::cout << "Error " << err << " going to first entry in archive\n";

  while (err == MZ_OK) {
    err = mz_zip_reader_entry_get_info(reader, &file_info);
    if (err != MZ_OK) {
      std::cout << "Error " << err << " getting info from archive\n";
      break;
    }

    /* Copy all entries from original archive to temporary archive
       except the ones we don't want */
    auto file_name = std::filesystem::path(file_info->filename);
    if (!isCriterionFile(file_name) && !isConstraintsFile(file_name) &&
        !isVariablesFile(file_name) && !isAreaFile(file_name) &&
        !isIntercoFile(file_name) && !IsAntaresMpsFile(file_name)) {
      err = mz_zip_writer_copy_from_reader(writer, reader);

      if (err != MZ_OK) {
        std::cout << "Error " << err << " copying entry into new zip\n";
        break;
      }
    }

    err = mz_zip_reader_goto_next_entry(reader);

    if (err != MZ_OK && err != MZ_END_OF_LIST)
      std::cout << "Error " << err << " going to next entry in archive\n";
  }

  mz_zip_reader_get_zip_cd(reader, &zip_cd);
  mz_zip_writer_set_zip_cd(writer, zip_cd);

  return err;
}

void AntaresArchiveUpdater::Update(ArchiveWriter &writer,
                                   const std::filesystem::path &path_to_add,
                                   const bool delete_path) {
  const auto root_path = path_to_add.parent_path();
  writer.AddPathInArchive(path_to_add, root_path);

  if (delete_path) {
    std::filesystem::remove_all(path_to_add);
  }
}

void AntaresArchiveUpdater::CleanAntaresArchive(
    const std::filesystem::path &src_archive,
    const std::filesystem::path &target_archive) {
  auto tmp_target_path = target_archive;
  auto reuse_src_path =
      (target_archive == "" || target_archive.compare(src_archive) == 0);
  if (reuse_src_path) {
    tmp_target_path =
        src_archive.parent_path() / (src_archive.stem().string() + ".tmp.zip");
  }

  auto reader = ArchiveReader(src_archive);
  auto writer = ArchiveWriter(tmp_target_path);
  reader.Open();
  writer.Open();
  MinizipErase(reader.InternalPointer(), writer.InternalPointer());
  reader.Close();
  reader.Delete();
  writer.Close();
  writer.Delete();
  if (reuse_src_path) {
    std::filesystem::copy(tmp_target_path, src_archive,
                          std::filesystem::copy_options::overwrite_existing);
    std::filesystem::remove(tmp_target_path);
  } else {
    std::filesystem::remove(src_archive);
  }
}