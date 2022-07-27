

#include <mz.h>

#include <fstream>

#include "ArchiveReader.h"
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
  fileExt.Open();
  fileExt.Close();
  fileExt.Delete();
  std::cerr.rdbuf(initialBufferCerr);
  ASSERT_EQ(redirectedErrorStream.str(), expectedErrorString.str());
}

TEST_F(ArchiveReaderTest, ShouldExtractFile1FromArchive1) {
  auto fileExt = ArchiveReader(archive1);
  ASSERT_EQ(fileExt.Open(), MZ_OK);
  const std::filesystem::path tmpDir = "/tmp/";
  const auto expectedFilePath = tmpDir / archive1File1.filename();
  fileExt.ExtractFile(archive1File1.filename(), tmpDir);
  ASSERT_TRUE(std::filesystem::exists(expectedFilePath));
  fileExt.Close();
  fileExt.Delete();
}
