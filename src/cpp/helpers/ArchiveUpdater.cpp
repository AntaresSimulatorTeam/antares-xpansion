#include "ArchiveUpdater.h"

#include <iostream>
#include <vector>

#include "ArchiveReader.h"

int32_t ArchiveUpdater::MinizipErase(
    const std::vector<std::filesystem::path> &paths, void *reader,
    void *writer) {
  mz_zip_file *file_info = NULL;
  const char *filename_in_zip = NULL;

  int32_t skip = 0;
  int32_t err = MZ_OK;
  uint8_t zip_cd = 0;

  std::vector<std::filesystem::path> unseen_paths;
  std::vector<std::filesystem::path> remaining_paths = paths;

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
    unseen_paths.clear();
    skip = 0;
    for (const auto &filename_in_zip : remaining_paths) {
      std::cout << "filename_in_zip = " << filename_in_zip << "\n";
      std::cout << "file_info->filename = " << file_info->filename << "\n";
      std::cout << "std::filesystem::path(file_info->filename) = "
                << std::filesystem::path(file_info->filename) << "\n";
      if (filename_in_zip.compare(file_info->filename) == 0) {
        skip = 1;
        std::cout << "skipped\n";
      } else {
        unseen_paths.push_back(filename_in_zip);
      }
    }
    remaining_paths = unseen_paths;
    if (skip == 0) {
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

void ArchiveUpdater::Update(ArchiveWriter &writer,
                            const std::filesystem::path &path_to_add,
                            const bool delete_path) {
  const auto root_path = path_to_add.parent_path();
  writer.AddPathInArchive(path_to_add, root_path);

  if (delete_path) {
    std::filesystem::remove_all(path_to_add);
  }
}

void ArchiveUpdater::DeleteFromArchive(
    const std::vector<std::filesystem::path> &paths,
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
  MinizipErase(paths, reader.InternalPointer(), writer.InternalPointer());
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