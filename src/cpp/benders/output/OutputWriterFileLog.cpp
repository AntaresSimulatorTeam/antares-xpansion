#pragma

#include "OutputWriterFileLog.h"

OutputWriterFileLog::OutputWriterFileLog(
    const std::filesystem::path& log_file) {
  file_stream_.open(log_file, std::ofstream::out | std::ofstream::app);
  if (file_stream_.fail()) {
    std::cerr << "Invalid file name passed as parameter" << std::endl;
  }
}
OutputWriterFileLog::~OutputWriterFileLog() {
  if (file_stream_.is_open()) {
    file_stream_.close();
  }
}
void OutputWriterFileLog::write_message(const std::string& message) {
  file_stream_ << message;
  file_.flush();
}