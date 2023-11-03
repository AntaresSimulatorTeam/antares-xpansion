#include "ProblemGeneration.h"
#include "ProblemGenerationExeOptions.h"
#include "gtest/gtest.h"

namespace po = boost::program_options;

class ProblemGenerationExeOptionsTest : public ::testing::Test {
 protected:
  ProblemGenerationExeOptions problem_generation_options_parser_;
};

class ProblemGenerationSpyAndMock : public ProblemGeneration {
 public:
  ProblemGenerationSpyAndMock(ProblemGenerationExeOptions& options)
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
  bool unnamed_problems_;
};

TEST_F(ProblemGenerationExeOptionsTest, WithoutArchiveOption) {
  char argv = 'c';
  char* pargv = &argv;
  char** ppargv = &pargv;
  try {
    problem_generation_options_parser_.Parse(1, ppargv);
  } catch (const std::exception& e) {
    EXPECT_EQ(e.what(),
              std::string("the option '--archive' is required but missing"));
  }
}
TEST_F(ProblemGenerationExeOptionsTest, WithoutOutputOption) {
  const char argv0[] = "lp_namer.exe";
  const char argv1[] = "--archive";
  const char argv2[] = "something";
  std::vector<const char*> ppargv = {argv0, argv1, argv2};
  try {
    problem_generation_options_parser_.Parse(3, ppargv.data());
  } catch (const std::exception& e) {
    EXPECT_EQ(e.what(),
              std::string("the option '--output' is required but missing"));
  }
}

TEST_F(ProblemGenerationExeOptionsTest, MasterFormulationDefaultValue) {
  const char argv0[] = "lp.exe ";
  const char argv1[] = "--archive";
  const char argv2[] = "something";
  const char argv3[] = "--output";
  const char argv4[] = "something";
  std::vector<const char*> ppargv = {argv0, argv1, argv2, argv3, argv4};
  problem_generation_options_parser_.Parse(5, ppargv.data());
  ASSERT_EQ(problem_generation_options_parser_.MasterFormulation(),
            std::string("relaxed"));
}

TEST_F(ProblemGenerationExeOptionsTest,
       OutputAndArchiveParameters_useProperValues) {
  auto test_root =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  auto archive = std::string(tmpnam(nullptr)) + "study.zip";
  auto output_path = test_root / "study-Xpansion";

  const char argv0[] = "lp.exe ";
  const char argv1[] = "--archive";
  auto argv2 = archive;
  const char argv3[] = "--output";
  auto argv4 = output_path.string();
  std::vector<const char*> ppargv = {argv0, argv1, argv2.c_str(), argv3,
                                     argv4.c_str()};
  problem_generation_options_parser_.Parse(5, ppargv.data());

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

  const char argv0[] = "lp.exe ";
  const char argv1[] = "--archive";
  auto argv2 = archive;

  std::vector<const char*> ppargv = {argv0, argv1, argv2.c_str()};
  problem_generation_options_parser_.Parse(3, ppargv.data());

  ProblemGenerationSpyAndMock pbg(problem_generation_options_parser_);
  pbg.updateProblems();

  EXPECT_EQ(pbg.archive_path_, archive);
  EXPECT_EQ(pbg.xpansion_output_dir_, output_path);
  EXPECT_TRUE(std::filesystem::exists(output_path));
  EXPECT_TRUE(std::filesystem::exists(output_path / "lp"));
}

TEST_F(ProblemGenerationExeOptionsTest,
       OutputAndArchiveParameters_deduceArchiveFromOutputDir) {
  auto test_root =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  auto archive = test_root / "study.zip";
  auto output_path = test_root / "study-Xpansion";

  const char argv0[] = "lp.exe ";
  const char argv1[] = "--output";
  auto argv2 = output_path.string();

  std::vector<const char*> ppargv = {argv0, argv1, argv2.c_str()};
  problem_generation_options_parser_.Parse(3, ppargv.data());

  ProblemGenerationSpyAndMock pbg(problem_generation_options_parser_);
  pbg.updateProblems();

  EXPECT_EQ(pbg.archive_path_, archive);
  EXPECT_EQ(pbg.xpansion_output_dir_, output_path);
  EXPECT_TRUE(std::filesystem::exists(output_path));
  EXPECT_TRUE(std::filesystem::exists(output_path / "lp"));
}

bool Constains(std::ifstream& file, const std::string& string) {
  std::string line;

  while (std::getline(file, line)) {
    if (line.find(string) != std::string::npos) {
      return true;
    }
  }
  return false;
}

TEST_F(
    ProblemGenerationExeOptionsTest,
    OutputAndArchiveParameters_cantDeduceArchiveFromOutputDirWithoutRegularSuffixe) {
  auto test_root =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  auto archive = test_root / "study.zip";
  auto output_path = test_root / "study-out";

  const char argv0[] = "lp.exe ";
  const char argv1[] = "--output";
  auto argv2 = output_path.string();

  std::vector<const char*> ppargv = {argv0, argv1, argv2.c_str()};
  problem_generation_options_parser_.Parse(3, ppargv.data());

  ProblemGenerationSpyAndMock pbg(problem_generation_options_parser_);

  EXPECT_THROW(pbg.updateProblems(),
               ProblemGenerationOptions::MismatchedParameters);

  EXPECT_TRUE(pbg.archive_path_.empty());  // Can't deduce
  EXPECT_TRUE(pbg.xpansion_output_dir_
                  .empty());  // Exception occurres before RunProblemGeneration
  // We expect to have created directories to log properly
  EXPECT_TRUE(std::filesystem::exists(output_path));
  EXPECT_TRUE(std::filesystem::exists(output_path / "lp"));

  std::ifstream logfile(output_path / "lp" / "ProblemGenerationLog.txt");
  EXPECT_TRUE(Constains(
      logfile, "Archive path is missing and output path does not contains"));
}