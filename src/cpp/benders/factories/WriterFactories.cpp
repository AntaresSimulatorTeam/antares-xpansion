

#include "WriterFactories.h"

#include "JsonWriter.h"
#include "OutputWriter.h"
#include "VoidWriter.h"

Writer build_void_writer() { return std::make_shared<Output::VoidWriter>(); }

Writer build_json_writer(const std::string &json_file_name) {
  auto m_clock = std::make_shared<Clock>();

  Writer writer = std::make_shared<Output::JsonWriter>(m_clock, json_file_name);

  writer->initialize();
  return writer;
}
