

#include "WriterFactories.h"

#include "JsonWriter.h"
#include "OutputWriter.h"
#include "VoidWriter.h"

Writer build_void_writer() { return std::make_shared<Output::VoidWriter>(); }

Writer build_json_writer(const BendersOptions &options) {
  auto m_clock = std::make_shared<Clock>();

  Writer writer =
      std::make_shared<Output::JsonWriter>(m_clock, options.JSON_FILE);

  writer->initialize(options);
  return writer;
}
