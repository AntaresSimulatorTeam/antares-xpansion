//
// Created by marechaljas on 10/10/23.
//

#include <filesystem>

#include "ArchiveIO.h"
#include "LoggerBuilder.h"
#include "ProblemGeneration.h"
#include "gtest/gtest.h"

class InMemoryOption : public ProblemGenerationOptions {
 public:
  std::filesystem::path XpansionOutputDir() const override {
    return std::filesystem::path();
  }
  std::string MasterFormulation() const override { return std::string(); }
  std::string AdditionalConstraintsFilename() const override {
    return std::string();
  }
  std::filesystem::path ArchivePath() const override {
    return std::filesystem::path();
  }
  std::filesystem::path WeightsFile() const override {
    return std::filesystem::path();
  }
  std::vector<int> ActiveYears() const override { return std::vector<int>(); }
  bool UnnamedProblems() const override { return false; }
};

TEST(InitializationTest, FoldersAreEmpty) {
  auto workingDirectory =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  auto simulationDirectory = workingDirectory / "output" / "simulation";
  auto logger = emptyLogger();
  std::filesystem::create_directories(simulationDirectory);

  auto xpansionDirectory =
      (simulationDirectory.parent_path() /
       (simulationDirectory.filename().string() + "-Xpansion"));
  std::filesystem::create_directories(xpansionDirectory);
  std::ofstream emptyfile(xpansionDirectory / "garbage.txt");

  InMemoryOption options;
  ProblemGeneration pbg(options);

  EXPECT_TRUE(std::filesystem::exists(xpansionDirectory / "garbage.txt"));
  EXPECT_THROW(
      pbg.RunProblemGeneration(simulationDirectory, "integer", "", "", logger,
                               simulationDirectory / "logs.txt", "", false),
      ArchiveIOGeneralException);

  EXPECT_FALSE(std::filesystem::exists(simulationDirectory / "garbage.txt"));
}