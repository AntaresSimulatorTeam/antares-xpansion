

#include <fstream>

#include "FileExtracter.h"
#include "gtest/gtest.h"

/*
ARCHIVES COMPOSITION:

data/mps_zip/archive1.zip
data/mps_zip/archive1
                    |__ file1
                    |__ file2
                    |__ file3

*/

const std::filesystem::path mpsZipDir = "data/mps_zip";
const std::filesystem::path archive1 = mpsZipDir / "archive1.zip";
const std::filesystem::path archive1Dir = mpsZipDir / "archive1";
const std::filesystem::path archive1File1 = archive1Dir / "file1";
const std::filesystem::path archive1File2 = archive1Dir / "file2";
const std::filesystem::path archive1File3 = archive1Dir / "file3";

class FileExtracterTest : public ::testing::Test {
 public:
  FileExtracterTest() = default;

  const std::filesystem::path invalid_file_path = "";
  const std::string invalid_file_name = "";
};

TEST_F(FileExtracterTest, ShouldFailIfInvalidFileIsGiven) {
  std::stringstream expectedErrorString;
  expectedErrorString << "unable to open archive: "
                      << invalid_file_path.string() << std::endl;

  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());
  auto fileExt = FileExtracter(invalid_file_path, invalid_file_name);
  fileExt.GetExctractedFilePath();
  std::cerr.rdbuf(initialBufferCerr);
  ASSERT_EQ(redirectedErrorStream.str(), expectedErrorString.str());
}

TEST_F(FileExtracterTest, ShouldExtractFile1FromArchive1) {
  auto fileExt = FileExtracter(archive1, archive1File1.filename());
  const auto fileExtPath = fileExt.GetExctractedFilePath();
  ASSERT_EQ(redirectedErrorStream.str(), expectedErrorString.str());
}
