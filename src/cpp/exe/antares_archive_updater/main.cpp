#include "ArchiveUpdater.h"
#include "ArchiveUpdaterExeOptions.h"

int main(int argc, char** argv) {
  ArchiveUpdaterExeOptions options_parser;

  options_parser.Parse(argc, argv);
  ArchiveUpdater::Update(options_parser.Archive(), options_parser.PathToAdd(),
                         options_parser.DeletePath());
  return 0;
}