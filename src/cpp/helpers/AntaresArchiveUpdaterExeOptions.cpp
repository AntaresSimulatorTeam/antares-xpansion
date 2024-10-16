#include "antares-xpansion/helpers/AntaresArchiveUpdaterExeOptions.h"

namespace po = boost::program_options;

AntaresArchiveUpdaterExeOptions::AntaresArchiveUpdaterExeOptions()
    : OptionsParser("Problem Generation exe") {
  AddOptions()("help,h", "produce help message")(
      "archive,a", po::value<std::filesystem::path>(&archive_)->required(),
      "Path to archive")(
      "paths-to-add,p",
      po::value<std::vector<std::filesystem::path>>(&paths_to_add_)
          ->required()
          ->multitoken(),
      "paths to add in the archive separated by whitespace")(
      "delete,d", po::bool_switch(&delete_path_),
      "add this option to delete files (or dir) specified by paths-to-add");
}