#include "FileInBuffer.h"

#include <fstream>
#include <iostream>
#include <sstream>

FileBuffer FileInBuffer::run(const std::filesystem::path& filePath) {
  std::ifstream file(filePath);
  std::stringstream buffer;
  if (file.bad()) {
    std::cerr << "Error while reading file : " << filePath.c_str() << std::endl;
    return {};
  }
  buffer << file.rdbuf();
  //   if (!keepFile_) {
  std::filesystem::remove(filePath);
  //   }
  return {filePath.filename().string(), buffer.str()};
}
