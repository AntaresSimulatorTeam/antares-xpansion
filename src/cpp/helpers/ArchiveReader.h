#ifndef _ARCHIVEREADER_H
#define _ARCHIVEREADER_H
#include "ArchiveIO.h"
class ArchiveReader : public ArchiveIO {
 private:
  void* internalPointer_ = NULL;
  void* handle_ = NULL;
  void Create() override;

 public:
  explicit ArchiveReader(const std::filesystem::path& archivePath);
  ArchiveReader();

  int32_t Close() override;
  void Delete() override;
  //   void* InternalPointer() const override;
  //   void* handle() const override;

  int Open() override;
  int32_t ExtractFile(const std::filesystem::path& FileToExtractPath);
  int32_t ExtractFile(const std::filesystem::path& FileToExtractPath,
                      const std::filesystem::path& destination);
};
#endif  // _ARCHIVEREADER_H
