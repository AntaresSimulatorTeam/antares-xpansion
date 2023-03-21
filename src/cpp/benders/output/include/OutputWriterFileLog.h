#pragma

#include <filesystem>
#include <sstream>

#include "IWriterLogger.h"

class OutputWriterFileLog : public IWriterLogger {
 public:
  explicit OutputWriterFileLog(const std::filesystem::path& log_file);
  void write_message(const std::string& message) override;
  virtual ~OutputWriterFileLog();

 private:
  std::ofstream file_stream_;
};