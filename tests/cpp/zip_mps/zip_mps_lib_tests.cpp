

#include <mz.h>

#include <fstream>

#include "ArchiveReader.h"
#include "ArchiveWriter.h"
#include "FileInBuffer.h"
#include "TempFileFactory.h"
#include "gtest/gtest.h"

/*
ARCHIVES COMPOSITION:

data/mps_zip/archive1.zip
data/mps_zip/archive1
                    |__ file1
                    |__ file2
                    |__ file3

*/

const std::filesystem::path mpsZipDir = "data_test/mps_zip";
const std::filesystem::path archive1 = mpsZipDir / "archive1.zip";
const std::filesystem::path archive1Dir = mpsZipDir / "archive1";
const std::filesystem::path archive1File1 = archive1Dir / "file1";
const std::filesystem::path archive1File2 = archive1Dir / "file2";
const std::filesystem::path archive1File3 = archive1Dir / "file3";

class ArchiveReaderTest : public ::testing::Test {
 public:
  ArchiveReaderTest() = default;

  const std::filesystem::path invalid_file_path = "";
  const std::string invalid_file_name = "";
};

TEST_F(ArchiveReaderTest, ShouldFailIfInvalidFileIsGiven) {
  std::stringstream expectedErrorString;
  expectedErrorString << "Could not Open Archive: "
                      << invalid_file_path.string() << std::endl;

  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());
  auto fileExt = ArchiveReader(invalid_file_path);
  ASSERT_EQ(fileExt.Open(), MZ_OPEN_ERROR);
  ASSERT_EQ(fileExt.Close(), MZ_OK);
  fileExt.Delete();
  std::cerr.rdbuf(initialBufferCerr);
  ASSERT_EQ(redirectedErrorStream.str(), expectedErrorString.str());
}

TEST_F(ArchiveReaderTest, ShouldExtractFile1FromArchive1) {
  auto fileExt = ArchiveReader(archive1);
  ASSERT_EQ(fileExt.Open(), MZ_OK);
  const std::filesystem::path tmpDir = "/tmp/";
  const auto expectedFilePath = tmpDir / archive1File1.filename();
  ASSERT_EQ(fileExt.ExtractFile(archive1File1.filename(), tmpDir), MZ_OK);
  ASSERT_TRUE(std::filesystem::exists(expectedFilePath));
  ASSERT_EQ(fileExt.Close(), MZ_OK);
  fileExt.Delete();
}
class ArchiveWriterTest : public ::testing::Test {
 public:
  ArchiveWriterTest() = default;
};

FileBufferVector GetRandomFileBufferVector(const size_t vecSize) {
  FileBufferVector result;
  for (size_t i = 0; i < vecSize; i++) {
    std::string fname = "file_" + std::to_string(i);
    result.push_back({fname, fname + " data"});
  }
  return result;
}

TEST_F(ArchiveWriterTest, ShouldCreateArchiveWithVecBuffer) {
  const std::filesystem::path archiveName = std::tmpnam(nullptr);
  ArchiveWriter writer(archiveName);
  ASSERT_EQ(writer.Open(), MZ_OK);
  ASSERT_EQ(writer.AddFilesInArchive(GetRandomFileBufferVector(5)), MZ_OK);
  ASSERT_EQ(writer.Close(), MZ_OK);
  writer.Delete();
}
class FileInBufferTest : public ::testing::Test {
 public:
  FileInBufferTest() = default;
};

TEST_F(FileInBufferTest, ShouldCreateReturnBufferVector) {
  char templatedFileName[10] = "ZIPXXXXXX";
  const auto ret = mktemp_platform(templatedFileName, 0);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  ASSERT_NE(ret, 0);
#else  // defined(__unix__) || (__APPLE__)
  ASSERT_GT(ret, 1);
#endif
}
