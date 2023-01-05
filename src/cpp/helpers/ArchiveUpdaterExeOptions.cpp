#include "ArchiveUpdaterExeOptions.h"

namespace po = boost::program_options;

ArchiveUpdaterExeOptions::ArchiveUpdaterExeOptions()
    : OptionsParser("Problem Generation exe") {
  AddOptions()("help,h", "produce help message")(
      "archive,a", po::value<std::filesystem::path>(&archive_)->required(),
      "Path to archive")(
      "path-to-add,p",
      po::value<std::filesystem::path>(&path_to_add_)->required(),
      "path to add in the archive")(
      "delete,d", po::bool_switch(&delete_path_),
      "add this option to delete files (or dir) specified by path-to-add");
}