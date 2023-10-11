//
// Created by marechaljas on 10/10/23.
//

#include <filesystem>

#include "ArchiveIO.h"
#include "LoggerBuilder.h"
#include "RunProblemGeneration.h"
#include "gtest/gtest.h"

TEST(InitializationTest, FoldersAreCreated) {
  auto workingDirectory =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  auto simulationDirectory = workingDirectory / "output" / "simulation";
  auto logger = emptyLogger();
  std::filesystem::create_directories(simulationDirectory);
  EXPECT_THROW(
      RunProblemGeneration(simulationDirectory, "integer", "", "", logger,
                           simulationDirectory / "logs.txt", "", false),
      ArchiveIOGeneralException);
  auto lp = (simulationDirectory.parent_path() /
       (simulationDirectory.filename().string() + "-Xpansion")) /
      "lp";
  EXPECT_TRUE(std::filesystem::exists(lp));
}

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

  EXPECT_TRUE(std::filesystem::exists(xpansionDirectory / "garbage.txt"));
  EXPECT_THROW(
      RunProblemGeneration(simulationDirectory, "integer", "", "", logger,
                           simulationDirectory / "logs.txt", "", false),
      ArchiveIOGeneralException);

  EXPECT_FALSE(std::filesystem::exists(simulationDirectory / "garbage.txt"));
}

TEST(InitializationTest, ErrorIfSimulationPathDoesNotExists) {
  auto workingDirectory =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  auto simulationDirectory = workingDirectory / "output" / "simulation";
  auto logger = emptyLogger();

  EXPECT_THROW(
      RunProblemGeneration(simulationDirectory, "integer", "", "", logger,
                           simulationDirectory / "logs.txt", "", false),
      LogUtils::XpansionError<std::runtime_error>);
  auto lp = (simulationDirectory.parent_path() /
             (simulationDirectory.filename().string() + "-Xpansion")) /
            "lp";
  EXPECT_FALSE(std::filesystem::exists(lp));
}