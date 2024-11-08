#include <fstream>

#include "LoggerBuilder.h"
#include "RandomDirGenerator.h"
#include "antares-xpansion/lpnamer/input_reader/SettingsReader.h"
#include "gtest/gtest.h"

TEST(ReadSettings, ReadSettingsProperly) {
  auto tmp_path = CreateRandomSubDir(std::filesystem::temp_directory_path());
  std::ofstream settings_file(tmp_path / "settings.ini");
  settings_file << R"(
      uc_type = expansion_fast
      master = integer
      optimality_gap = 10
      additional-constraints = contraintes.txt
      solver = Xpress
    )" << std::endl;
  auto logger = emptyLogger();
  ASSERT_NO_THROW(SettingsReader reader(tmp_path / "settings.ini", logger.get()));
}

TEST(ReadSettings, DefaultSolverIsCBC) {
        auto tmp_path = CreateRandomSubDir(std::filesystem::temp_directory_path());
        std::ofstream settings_file(tmp_path / "settings.ini");
        settings_file << R"(
        uc_type = expansion_fast
        master = integer
        optimality_gap = 10
        additional-constraints = contraintes.txt
        )"<< std::endl;;
        auto logger = emptyLogger();
        SettingsReader reader(tmp_path / "settings.ini", logger.get());
        ASSERT_EQ(reader.Solver(), "CBC");
}

TEST(ReadSettings, ReadSolverProperly) {
        auto tmp_path = CreateRandomSubDir(std::filesystem::temp_directory_path());
        std::ofstream settings_file(tmp_path / "settings.ini");
        settings_file << R"(
        uc_type = expansion_fast
        master = integer
        optimality_gap = 10
        additional-constraints = contraintes.txt
        solver = Xpress
        )"<< std::endl;
        auto logger = emptyLogger();
        SettingsReader reader(tmp_path / "settings.ini", logger.get());
        ASSERT_EQ(reader.Solver(), "Xpress");
}