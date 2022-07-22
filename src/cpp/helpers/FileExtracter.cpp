#include "FileExtracter.h"

#include <fstream>
#include <iostream>
FileExtracter::FileExtracter(const std::filesystem::path& archiveName,
                             const std::string& fileName)
    : archiveName_(archiveName), fileName_(fileName), archive_(archiveName_) {}

std::filesystem::path FileExtracter::GetExctractedFilePath() {
  if (OpenArchive_()) {
    auto entry = archive_.getEntry(fileName_);
    const auto extractedFileFullPath(archiveName_.parent_path() / fileName_);
    std::ofstream extractedFile(extractedFileFullPath);
    if (const auto err = entry.readContent(extractedFile) != LIBZIPPP_OK) {
      std::cerr << "error while extracting file: " << fileName_
                << " from archive: " << archiveName_.string() << std::endl
                << "error number: " << err << std::endl;
    } else {
      return extractedFileFullPath;
    }
  }

  std::cerr << "unable to open archive: " << archiveName_.string() << std::endl;
  return "";
}
