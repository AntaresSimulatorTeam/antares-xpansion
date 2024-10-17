#include "antares-xpansion/helpers/FileInBuffer.h"

#include <fstream>
#include <iostream>
#include <sstream>

FileBuffer FileInBuffer::run(const std::filesystem::path& filePath) {
  std::ifstream file(filePath);
  std::stringstream buffer;
  if (file.bad()) {
    std::cerr << "Error while reading file : " << filePath << std::endl;
    return {};
  }
  buffer << file.rdbuf();
  file.close();
  //   if (!keepFile_) {
  std::filesystem::remove(filePath);
  //   }
  return {filePath.filename().string(), buffer.str()};
}
