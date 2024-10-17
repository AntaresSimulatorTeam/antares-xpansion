#include <fstream>

#include "antares-xpansion/lpnamer/input_reader/WeightsFileWriter.h"
#include "gtest/gtest.h"

void writeDummyFileInTempLpDir(std::string name) {
  auto tmp_path = std::filesystem::temp_directory_path() / "lp" / name;
  std::ofstream writer;
  writer.open(tmp_path);
  writer << std::endl;
  writer.close();
}

TEST(WeightsFileWriterTest, CorrectlyWriteWeightsFile) {
  auto tempDir = std::filesystem::temp_directory_path() / "lp";
  if (!std::filesystem::is_directory(tempDir)) {
    std::filesystem::create_directory(tempDir);
  }
  writeDummyFileInTempLpDir("problem-1-1--optim-nb-1.mps");
  writeDummyFileInTempLpDir("problem-1-50--optim-nb-1.mps");
  writeDummyFileInTempLpDir("problem-2-10--optim-nb-1.mps");
  writeDummyFileInTempLpDir("problem-2-11--optim-nb-1.mps");
  writeDummyFileInTempLpDir("problem-2-30--optim-nb-1.mps");
  writeDummyFileInTempLpDir("problem-3-20--optim-nb-1.mps");
  writeDummyFileInTempLpDir("problem-3-30--optim-nb-1.txt");

  auto yearly_weight_writer =
      YearlyWeightsWriter(std::filesystem::temp_directory_path(), {3, 5, 7},
                          "weights_123.txt", {1, 2, 3});
  yearly_weight_writer.CreateWeightFile();

  std::ifstream reader(tempDir / "weights_123.txt");
  std::string actual((std::istreambuf_iterator<char>(reader)),
                     std::istreambuf_iterator<char>());
  std::string expected = R"xxx(problem-1-1--optim-nb-1.mps 3
problem-1-50--optim-nb-1.mps 3
problem-2-10--optim-nb-1.mps 5
problem-2-11--optim-nb-1.mps 5
problem-2-30--optim-nb-1.mps 5
problem-3-20--optim-nb-1.mps 7
WEIGHT_SUM 15
)xxx";

  EXPECT_EQ(expected, actual);
}