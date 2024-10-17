#pragma once
#include <string>

class IWriterLogger {
  virtual void write_message(const std::string& message) = 0;
};