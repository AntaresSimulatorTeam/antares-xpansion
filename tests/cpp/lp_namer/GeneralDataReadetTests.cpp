#include <gtest/gtest.h>

#include "antares-xpansion/lpnamer/input_reader/GeneralDataReader.h"

//Test for the GeneralDataIniReader class
class GeneralDataIniReaderTests : public testing::TestWithParam<bool> {
 public:
  GeneralDataIniReaderTests() = default;
  ~GeneralDataIniReaderTests() override = default;
};

INSTANTIATE_TEST_SUITE_P(reset_value, GeneralDataIniReaderTests,
                         testing::Bool());
TEST_P(GeneralDataIniReaderTests, ReadPlaylistReset_option) {
  auto test_dir = std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  std::filesystem::create_directory(test_dir);
  auto ini_file = test_dir / "test.ini";
  std::ofstream file(ini_file);
  file << "[general]\n"
          "nbyears=10\n"
          "user-playlist=true\n"
          "[playlist]\n"
          "playlist_reset = ";
  file << (GetParam() ? "true\n" : "false\n");
  file << "playlist_year += 0\n"
          "playlist_year += 5\n"
          "playlist_year += 8\n"
          "playlist_year -= 1\n"
          "playlist_year -= 2\n"
          "playlist_year -= 3\n";
  file.close();

  GeneralDataIniReader reader(ini_file, nullptr);
  //GetActiveYears() returns the active years whereas file contains index. year=index+1
  //reset = false => Active playlist (+=)
  //rest = true => !inactive playlist (-=)
  if (GetParam()) {
    EXPECT_EQ(reader.GetActiveYears(), std::vector<int>({1, 5, 6, 7, 8, 9, 10}));
  } else {
    EXPECT_EQ(reader.GetActiveYears(), std::vector<int>({1, 6, 9}));
  }
}