#include <concepts>
#include <string_view>

#include "ProblemGeneration.h"
#include "ProblemGenerationExeOptions.h"
#include "gtest/gtest.h"

namespace po = boost::program_options;

class ProblemGenerationExeOptionsTest : public ::testing::Test {
 public:
  ProblemGenerationExeOptions problem_generation_options_parser_;

  auto parseOptions(std::convertible_to<std::string_view> auto&&... s) {
    const char argv0[] = "lp_namer.exe";
    std::vector<const char*> ppargv;
    ppargv.push_back(argv0);
    for (auto v : std::initializer_list<std::string_view>{s...}) {
      ppargv.push_back(v.data());
    }
    return problem_generation_options_parser_.Parse(ppargv.size(),
                                                    ppargv.data());
  }
};

class ProblemGenerationSpyAndMock : public ProblemGeneration {
 public:
  virtual ~ProblemGenerationSpyAndMock() = default;
  explicit ProblemGenerationSpyAndMock(ProblemGenerationExeOptions& options)
      : ProblemGeneration(options) {}
  void RunProblemGeneration(
      const std::filesystem::path& xpansion_output_dir,
      const std::string& master_formulation,
      const std::string& additionalConstraintFilename_l,
      const std::filesystem::path& archive_path,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
      const std::filesystem::path& log_file_path,
      const std::filesystem::path& weights_file,
      bool unnamed_problems) override {
    xpansion_output_dir_ = xpansion_output_dir;
    master_formulation_ = master_formulation;
    additionalConstraintFilename_l_ = additionalConstraintFilename_l;
    archive_path_ = archive_path;
    log_file_path_ = log_file_path;
    weights_file_ = weights_file;
    unnamed_problems_ = unnamed_problems;
  }
  std::filesystem::path xpansion_output_dir_;
  std::string master_formulation_;
  std::string additionalConstraintFilename_l_;
  std::filesystem::path archive_path_;
  std::filesystem::path log_file_path_;
  std::filesystem::path weights_file_;
  bool unnamed_problems_ = false;
};

TEST_F(ProblemGenerationExeOptionsTest, WithoutOutputOption) {
  EXPECT_NO_THROW(parseOptions("--archive", "something"););
}

// OK
TEST_F(ProblemGenerationExeOptionsTest, MasterFormulationDefaultValue) {
  parseOptions("--output", "something");
  ASSERT_EQ(problem_generation_options_parser_.MasterFormulation(),
            std::string("relaxed"));
}

TEST_F(ProblemGenerationExeOptionsTest,
       OutputAndArchiveParameters_mutually_exclusives) {
  auto test_root =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  auto archive = std::string(tmpnam(nullptr)) + "study.zip";
  auto output_path = test_root / "study-Xpansion";

  EXPECT_THROW(
      parseOptions("--archive", archive, "--output", output_path.string()),
      ProblemGenerationOptions::ConflictingParameters);

  ProblemGenerationSpyAndMock pbg(problem_generation_options_parser_);
  pbg.updateProblems();

  EXPECT_EQ(pbg.archive_path_, archive);
  EXPECT_EQ(pbg.xpansion_output_dir_, output_path);
  EXPECT_TRUE(std::filesystem::exists(output_path));
  EXPECT_TRUE(std::filesystem::exists(output_path / "lp"));
}

TEST_F(ProblemGenerationExeOptionsTest,
       OutputAndArchiveParameters_deduceOuputFromArchive) {
  auto test_root =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  auto archive = test_root / "study.zip";
  auto output_path = test_root / "study-Xpansion";

  parseOptions("--archive", archive.string());

  ProblemGenerationSpyAndMock pbg(problem_generation_options_parser_);
  pbg.updateProblems();

  EXPECT_TRUE(problem_generation_options_parser_.XpansionOutputDir().empty());
  EXPECT_EQ(pbg.archive_path_, archive);
  EXPECT_EQ(pbg.xpansion_output_dir_, output_path);
  EXPECT_TRUE(std::filesystem::exists(output_path));
  EXPECT_TRUE(std::filesystem::exists(output_path / "lp"));
}

TEST_F(ProblemGenerationExeOptionsTest, use_only_output_option) {
  auto test_root =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  auto archive = test_root / "study.zip";
  auto output_path = test_root / "study-Xpansion";

  parseOptions("--output", output_path.string());

  ProblemGenerationSpyAndMock pbg(problem_generation_options_parser_);
  pbg.updateProblems();

  EXPECT_TRUE(problem_generation_options_parser_.ArchivePath().empty());
  EXPECT_TRUE(pbg.archive_path_.empty());
  EXPECT_EQ(pbg.xpansion_output_dir_, output_path);
  EXPECT_TRUE(std::filesystem::exists(output_path));
  EXPECT_TRUE(std::filesystem::exists(output_path / "lp"));
}

TEST_F(ProblemGenerationExeOptionsTest,
       OutputAndArchiveParameters_ErrorIfBothArchiveAndOutputAreMissing) {
  auto test_root =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);

  EXPECT_THROW(parseOptions(), ProblemGenerationOptions::MissingParameters);
}