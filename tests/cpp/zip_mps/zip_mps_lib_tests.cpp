

#include <mz.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

#include <algorithm>
#include <fstream>
#include <iterator>

#include "../../../src/cpp/archive_handler/include/antares-xpansion/archive_handler/ArchiveReader.h"
#include "RandomDirGenerator.h"
#include "antares-xpansion/archive_handler/ArchiveWriter.h"
#include "antares-xpansion/archive_handler/FileInBuffer.h"
#include "antares-xpansion/archive_updater/AntaresArchiveUpdater.h"
#include "gtest/gtest.h"

/*
ARCHIVES COMPOSITION:

data/mps_zip/archive1.zip
data/mps_zip/archive2.zip
data/mps_zip/archive1
                    |__ file1
                    |__ file2
                    |__ file3
data/mps_zip/archive2.zip
                    |__ file1.txt
                    |__ file2
                    |__ file3.xlsx
                    |__ file4.txt

*/

const auto mpsZipDir = std::filesystem::path("data_test") / "mps_zip";
const auto archive1 = mpsZipDir / "archive1.zip";
const auto archive2 = mpsZipDir / "archive2.zip";
const auto archive1Dir = mpsZipDir / "archive1";
const auto archive1File1 = archive1Dir / "file1";
const auto archive1File2 = archive1Dir / "file2";
const auto archive1File3 = archive1Dir / "file3";

class ArchiveReaderTest : public ::testing::Test {
 public:
  ArchiveReaderTest() = default;

  const std::filesystem::path invalid_file_path = "";
  const std::string invalid_file_name = "";
};

TEST_F(ArchiveReaderTest, ShouldFailIfInvalidFileIsGiven) {
  std::stringstream expectedErrorString;
  expectedErrorString << "Failed to Open Archive: "
                      << invalid_file_path.string() << std::endl
                      << " invalid status: " + std::to_string(MZ_OPEN_ERROR) +
                             " (" + std::to_string(MZ_OK) + " expected)";

  try {
    auto archive_reader = ArchiveReader(invalid_file_path);
  } catch (const ArchiveIOGeneralException& e) {
    EXPECT_EQ(e.what(), expectedErrorString.str());
  }
}
bool equal_files(const std::filesystem::path& a,
                 const std::filesystem::path& b) {
  std::ifstream stream{a};
  std::string file1{std::istreambuf_iterator<char>(stream),
                    std::istreambuf_iterator<char>()};

  stream = std::ifstream{b};
  std::string file2{std::istreambuf_iterator<char>(stream),
                    std::istreambuf_iterator<char>()};

  return file1 == file2;
}
TEST_F(ArchiveReaderTest, ShouldExtractFile1FromArchive1) {
  auto archive_reader = ArchiveReader(archive1);
  ASSERT_EQ(archive_reader.Open(), MZ_OK);
  const auto tmpDir = std::filesystem::temp_directory_path();
  const auto expectedFilePath = tmpDir / archive1File1.filename();
  ASSERT_EQ(archive_reader.ExtractFile(archive1File1.filename(), tmpDir),
            MZ_OK);
  ASSERT_TRUE(std::filesystem::exists(expectedFilePath));
  ASSERT_TRUE(equal_files(expectedFilePath, archive1File1));
  ASSERT_EQ(archive_reader.Close(), MZ_OK);
  archive_reader.Delete();
}
TEST_F(ArchiveReaderTest, ShouldReturnNumberOfFilesInGivenArchive) {
  auto archive_reader = ArchiveReader(archive1);
  ASSERT_EQ(archive_reader.Open(), MZ_OK);
  ASSERT_EQ(archive_reader.GetNumberOfEntries(), 3);
  ASSERT_EQ(archive_reader.Close(), MZ_OK);
  archive_reader.Delete();
}
TEST_F(ArchiveReaderTest, Shouldfindarchive1File2) {
  auto archive_reader = ArchiveReader(archive1);
  ASSERT_EQ(archive_reader.Open(), MZ_OK);
  archive_reader.LoadEntriesPath();
  auto file_names_vect = archive_reader.EntriesPath();
  ASSERT_TRUE(std::find(file_names_vect.begin(), file_names_vect.end(),
                        archive1File2.filename()) != file_names_vect.end());
  ASSERT_EQ(archive_reader.Close(), MZ_OK);
  archive_reader.Delete();
}
TEST_F(ArchiveReaderTest, ShouldReturnFilesWithExt_TXT) {
  auto archive_reader = ArchiveReader(archive2);
  ASSERT_EQ(archive_reader.Open(), MZ_OK);
  archive_reader.LoadEntriesPath();
  auto file_names_vect = archive_reader.GetEntriesPathWithExtension(".txt");
  ASSERT_EQ(file_names_vect.size(), 2);
  for (auto& file_path : file_names_vect) {
    ASSERT_TRUE(file_path.extension() == ".txt");
  }
  ASSERT_EQ(archive_reader.Close(), MZ_OK);
  archive_reader.Delete();
}
class ArchiveWriterTest : public ::testing::Test {
 public:
  ArchiveWriterTest() = default;
};

FileBufferVector GetBufferVectorOfFilesInDir(const std::filesystem::path& dir) {
  FileBufferVector result;
  for (const auto& file : std::filesystem::directory_iterator(dir)) {
    const auto& pathToFile = file.path();
    std::ifstream fileStream(pathToFile);
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    result.push_back({pathToFile.filename().string(), buffer.str()});
  }
  return result;
}
void compareArchiveAndDir(const std::filesystem::path& archivePath,
                          const std::filesystem::path& dirPath,
                          const std::filesystem::path& tmpDir) {
  void* reader = NULL;

  reader = mz_zip_reader_create();
  const auto& archive_path_str = archivePath.string();
  auto archive_path_c_str = archive_path_str.c_str();
  assert(mz_zip_reader_open_file(reader, archive_path_c_str) == MZ_OK);

  for (const auto& file : std::filesystem::directory_iterator(dirPath)) {
    const auto& filename_path = file.path().filename();
    const auto& filename_str = filename_path.string();
    const auto searchFilename = filename_str.c_str();
    assert(mz_zip_reader_locate_entry(reader, searchFilename, 1) == MZ_OK);
    assert(mz_zip_reader_entry_open(reader) == MZ_OK);
    const auto extractedFilePath = tmpDir / searchFilename;
    mz_zip_reader_entry_save_file(reader, extractedFilePath.string().c_str());
    assert(equal_files(extractedFilePath, file.path()));
  }
  mz_zip_reader_close(reader);
  mz_zip_reader_delete(&reader);
}
TEST_F(ArchiveWriterTest, ShouldCreateArchiveWithVecBuffer) {
  const auto tmpArchiveRoot = std::filesystem::temp_directory_path();
  const auto tmpDir =
      CreateRandomSubDir(std::filesystem::temp_directory_path());
  std::string archiveName = GenerateRandomString(6);
  archiveName += ".zip";
  const auto archivePath = tmpDir / archiveName;
  ArchiveWriter writer(archivePath);
  ASSERT_EQ(writer.Open(), MZ_OK);
  const auto mpsBufferVec = GetBufferVectorOfFilesInDir(archive1Dir);
  for (const auto& mpsBuf : mpsBufferVec) {
    ASSERT_EQ(writer.AddFileInArchive(mpsBuf), MZ_OK);
  }
  ASSERT_EQ(writer.Close(), MZ_OK);
  writer.Delete();
  compareArchiveAndDir(archivePath, archive1Dir, tmpDir);
}
class ArchiveUpdaterTest : public ::testing::Test {
 public:
  ArchiveUpdaterTest() = default;
};

TEST_F(ArchiveUpdaterTest, ThatNewFileAndDirCanBeAddToArchive) {
  auto tmp_dir = std::filesystem::temp_directory_path();
  auto new_dir_to_add = tmp_dir / "new_dir_to_add";
  std::filesystem::create_directory(new_dir_to_add);

  std::filesystem::copy(archive1, tmp_dir,
                        std::filesystem::copy_options::overwrite_existing);
  // create file in tmp/new_file1.txt
  auto new_file_name1 = tmp_dir / "new_file1.txt";
  std::ofstream new_file1(new_file_name1);
  auto new_file_content = "HELLO!";
  new_file1 << new_file_content;
  new_file1.close();

  // create another file in new_dir_to_add/
  auto new_file_name2 = new_dir_to_add / "file.m";
  std::filesystem::copy(new_file_name1, new_file_name2,
                        std::filesystem::copy_options::overwrite_existing);
  auto copy_archive = tmp_dir / archive1.filename();

  auto writer = ArchiveWriter(copy_archive);
  writer.Open();

  // adding new_file_name1 in archive
  AntaresArchiveUpdater::Update(writer, new_file_name1, true);
  // adding new_dir_to_add in archive
  AntaresArchiveUpdater::Update(writer, new_dir_to_add, true);
  writer.Close();
  writer.Delete();
  ASSERT_FALSE(std::filesystem::exists(new_file_name1));
  ASSERT_FALSE(std::filesystem::exists(new_dir_to_add));

  auto reader = ArchiveReader(copy_archive);
  reader.Open();
  auto file1_name_in_archive = new_file_name1.filename();
  auto string_stream = reader.ExtractFileInStringStream(file1_name_in_archive);
  ASSERT_STREQ(string_stream.str().c_str(), new_file_content);

  auto file2_name_in_archive =
      new_file_name2.parent_path().filename() / new_file_name2.filename();
  auto string_stream2 = reader.ExtractFileInStringStream(file2_name_in_archive);
  ASSERT_STREQ(string_stream2.str().c_str(), new_file_content);
  reader.Close();
  reader.Delete();
}