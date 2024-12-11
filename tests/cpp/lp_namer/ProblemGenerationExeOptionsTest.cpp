#include <concepts>
#include <string_view>

#include "antares-xpansion/lpnamer/main/ProblemGeneration.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"
#include "gtest/gtest.h"

namespace po = boost::program_options;

class ProblemGenerationExeOptionsTest
    : public testing::TestWithParam<
          std::tuple<std::pair<std::string, std::string>,
                     std::pair<std::string, std::string>>> {
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

 private:
  std::filesystem::path performAntaresSimulation() override {
    return options_.StudyPath() / "simulation";
  }
  void set_solver(
      std::filesystem::path study_dir,
      ProblemGenerationLog::ProblemGenerationLogger* logger) override {

  }

 public:
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

// Base case: an empty tuple
template <typename... Ts>
auto flattenPairs(const std::tuple<Ts...>& tuple) {
  return tuple;  // Return the original tuple for the base case
}

// Terminal case : only a pair
template <typename T>
auto flattenPairs(const std::pair<T, T>& pair) {
  return std::tuple(pair.first, pair.second);
}

// Recursive case: process the first pair and concatenate it with the flattened
// rest of the tuple f({ pair(A, B), pair(C, D), pair(E, F) })
// => f( pair(A,B) ) + f( { pair(C, D), pair(E, F) })
// => f( pair(A,B) ) + f( pair(C, D) ) + f ({ pair(E, F) })
// => f( pair(A,B) ) + f( pair(C, D) ) + f ( pair(E, F) ) + f ( {} )
template <typename T1, typename T2, typename... Ts>
auto flattenPairs(const std::pair<T1, T2>& pair, const Ts&... rest) {
  return std::tuple_cat(std::tuple(pair.first, pair.second),
                        flattenPairs(rest...));
}

auto cases() {
  using namespace std::string_literals;
  return ::testing::Combine(
      ::testing::Values(std::pair("--archive"s, "archive.zip"s),
                        std::pair("--output"s, "output-Xpansion"s),
                        std::pair("--study"s, "the_study"s)),
      ::testing::Values(std::pair("--archive"s, "archive.zip"s),
                        std::pair("--output"s, "output-Xpansion"s),
                        std::pair("--study"s, "the_study"s)));
}
TEST_P(ProblemGenerationExeOptionsTest, Parameters_mutually_exclusives) {
  auto test_root =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  auto archive = std::string(tmpnam(nullptr)) + "study.zip";
  auto output_path = test_root / "study-Xpansion";

  auto params = GetParam();
  if (std::get<0>(params) == std::get<1>(params)) {
    GTEST_SKIP() << "Same parameters";
  }
  auto tuple =
      std::apply([](auto... args) { return flattenPairs(args...); }, params);
  EXPECT_THROW(
      std::apply([&, this](auto... args) { parseOptions(args...); }, tuple),
      ProblemGenerationOptions::ConflictingParameters);

  EXPECT_THROW(
      parseOptions("--archive", archive, "--output", output_path.string(),
                   "--study", test_root.string()),
      ProblemGenerationOptions::ConflictingParameters);
}
INSTANTIATE_TEST_SUITE_P(ConflictingOptions, ProblemGenerationExeOptionsTest,
                         cases());

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

TEST_F(ProblemGenerationExeOptionsTest,
       ValuesAndXpansionDirExistsWhenUsingOutputOption) {
  auto test_root =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  auto simulation_path = test_root / "study";

  parseOptions("--output", simulation_path.string());

  ProblemGenerationSpyAndMock pbg(problem_generation_options_parser_);
  pbg.updateProblems();

  EXPECT_TRUE(problem_generation_options_parser_.ArchivePath().empty());
  EXPECT_TRUE(pbg.archive_path_.empty());
  EXPECT_EQ(pbg.xpansion_output_dir_, simulation_path.string());
  EXPECT_TRUE(std::filesystem::exists(pbg.xpansion_output_dir_ / "lp"));
}

TEST_F(ProblemGenerationExeOptionsTest,
       OutputAndArchiveParameters_ErrorIfBothArchiveAndOutputAreMissing) {
  auto test_root =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);

  EXPECT_THROW(parseOptions(), ProblemGenerationOptions::MissingParameters);
}

TEST_F(ProblemGenerationExeOptionsTest, study) {
  auto test_root =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);

  parseOptions("--study", test_root.string());

  ProblemGenerationSpyAndMock pbg(problem_generation_options_parser_);
  pbg.updateProblems();

  EXPECT_TRUE(problem_generation_options_parser_.ArchivePath().empty());
  EXPECT_TRUE(pbg.archive_path_.empty());
  EXPECT_TRUE(std::filesystem::exists(pbg.xpansion_output_dir_));
}
