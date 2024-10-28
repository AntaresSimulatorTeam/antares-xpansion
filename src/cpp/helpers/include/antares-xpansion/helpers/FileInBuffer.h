#ifndef _FILEINBUFFER_H
#define _FILEINBUFFER_H
#include <filesystem>
#include <vector>

struct FileBuffer {
  std::string fname;
  std::string buffer;
};
using FileBufferVector = std::vector<FileBuffer>;

class FileInBuffer {
 private:
  //   std::filesystem::path filePath_;
  //   bool keepFile_ = false;

 public:
  FileInBuffer() = default;
  //   explicit FileInBuffer(bool keepFile) : keepFile_(keepFile) {}

  FileBuffer run(const std::filesystem::path& filePath);
};
#endif  //_FILEINBUFFER_H